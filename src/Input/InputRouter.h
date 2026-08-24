#pragma once

#include "InputPrerequisites.h"
#include "System/EnginePrerequisites.h"

#include <vector>
#include <unordered_map>

namespace AV{
    class InputManager;
    class GuiInputProcessor;
    class InputMapper;
    class Window;

    typedef uint16 InputLayerHandle;
    static const InputLayerHandle INVALID_INPUT_LAYER = 0;

    //Higher priorities are offered input first.
    //Plain ints rather than an enum so a layer can slot between two of these
    //without the values themselves having to change.
    static const int INPUT_PRIORITY_GAME = 0; //The InputManager sink. Always last.
    static const int INPUT_PRIORITY_GUI = 100; //Colibri.
    static const int INPUT_PRIORITY_OVERLAY = 200; //Debug overlays, i.e. imgui.
    static const int INPUT_PRIORITY_SYSTEM = 300; //Engine hotkeys.

    //Scancodes are raw SDL values everywhere they cross into the engine, and
    //the router can't include SDL itself, so the two it acts on are named here.
    static const int INPUT_SCANCODE_F1 = 58;
    static const int INPUT_SCANCODE_F2 = 59;

    /**
    The callbacks a layer supplies to the router.

    Raw function pointers with a userData rather than a virtual interface,
    because layers are expected to live in dynamically loaded plugins which
    resolve engine symbols at load time. A virtual interface would tie those
    plugins to the engine's vtable layout.

    Every member may be null, which the router reads as "pass through".
    */
    struct InputLayerCallbacks{
        /**
        Whether the layer is currently in a position to answer at all.
        A layer whose backing system has no live frame returns false and is
        skipped entirely, making it transparent.

        Deliberately not consulted when routing an event to a layer which
        already owns it. A layer that goes non-live mid-drag must still be told
        about the release, otherwise it holds that button down forever.
        */
        bool (*isLive)(void* userData);

        /**
        Queries, not events. Used to pick the pointer focus and to drive the
        window's text input state. Must be free of side effects, as the router
        calls them more than once per event.
        */
        bool (*wantsPointer)(void* userData);
        bool (*wantsKeyboard)(void* userData);
        bool (*wantsTextInput)(void* userData);

        /**
        Mouse motion, which is never consumed. Position is state rather than an
        event: swallowing it would freeze the cursor for every layer below,
        which is far more visible than the input bleed this router exists to fix.

        @param x, y
        Window logical pixels.
        @param normX, normY
        The same position between 0 and 1.
        @param focused
        Whether this layer currently owns the pointer. A layer that draws hover
        feedback should clear it when this is false.
        */
        void (*onMouseMove)(float x, float y, float normX, float normY, bool focused, void* userData);

        //Return true to consume, denying the event to every lower layer.
        bool (*onMouseButton)(int button, bool pressed, void* userData);
        bool (*onMouseWheel)(float x, float y, void* userData);
        bool (*onKey)(int scancode, int keycode, int keyMod, bool pressed, void* userData);
        bool (*onTextInput)(const char* text, void* userData);
        bool (*onTextEdit)(const char* text, int selectStart, int selectLength, void* userData);

        /**
        Everything this layer held has been revoked, because the window lost
        focus, the display mode changed, or the layer is going away. Reset all
        held state.
        */
        void (*onInputCancelled)(void* userData);
        /**
        One shot notification that this layer no longer owns the pointer.
        */
        void (*onPointerFocusLost)(void* userData);
    };

    /**
    The single arbitration point for hardware input.

    Input used to be handed to the gui system and to the InputManager directly
    and unconditionally, which meant a click on a debug overlay also drove the
    game underneath it. Everything now passes through here instead, where layers
    are offered each event in priority order and may consume it.

    Both the window's event poll and the debug server's input playback must
    inject exclusively through this class. Anything which bypasses it leaves
    capture ownership out of sync with what the layers actually believe.

    Main thread only; all injection happens during Base::update.
    */
    class InputRouter{
    public:
        InputRouter();
        ~InputRouter();

        /**
        @param guiInputProcessor
        May be null, in which case the gui layer is transparent. Unit tests rely
        on this to exercise the router without a window or Colibri.
        @param window
        May be null. Supplies the logical size used to normalise coordinates.
        */
        void initialise(InputManager* inputManager, GuiInputProcessor* guiInputProcessor, Window* window);

        InputLayerHandle addLayer(const char* name, int priority, const InputLayerCallbacks& callbacks, void* userData);
        /**
        Remove a layer. Anything it owned is revoked first.
        Note that ownership is not handed down to lower layers, as they never saw
        the press which established it.
        */
        void removeLayer(InputLayerHandle handle);
        void setLayerEnabled(InputLayerHandle handle, bool enabled);
        bool isLayerEnabled(InputLayerHandle handle) const;

        /**
        Global escape hatch. When false only the engine's own layers run, so a
        project can disable all plugin input consumption without unloading them.
        */
        void setExternalLayersEnabled(bool enabled);
        bool getExternalLayersEnabled() const { return mExternalLayersEnabled; }

        /**
        Whether the gui system consumes the input it intersects.
        False by default, which preserves the engine's original behaviour of
        letting presses fall through to the game even when they land on a
        widget. Projects which want gui clicks to block gameplay opt in here.
        */
        void setGuiConsumesInput(bool consume) { mGuiConsumes = consume; }
        bool getGuiConsumesInput() const { return mGuiConsumes; }

        //The only legal way into the input system.
        void injectMouseMove(float x, float y); //Window logical pixels.
        void injectMouseButton(int button, bool pressed); //0 left, 1 right, 2 middle.
        void injectMouseWheel(float x, float y);
        void injectKey(int scancode, int keycode, int keyMod, bool pressed);
        void injectTextInput(const char* text);
        void injectTextEdit(const char* text, int selectStart, int selectLength);

        /**
        Pass through only. No layer arbitrates these yet, but they are routed
        here so there stays exactly one dispatch point to extend later.
        */
        void injectControllerAxis(InputMapper* mapper, InputDeviceId device, int axis, float value, bool isTrigger);
        void injectControllerButton(InputMapper* mapper, InputDeviceId device, int button, bool pressed);

        /**
        Release and revoke everything currently held.
        Used when the window loses focus or changes display mode, either of which
        can otherwise leave a key or button stuck down forever.
        */
        void cancelAllInput();

        /**
        Whether any layer currently wants typed text, which decides if the window
        enables text input events.
        */
        bool shouldTextInputEnable() const;

        //Reported by the debug server so the layer stack is observable.
        struct LayerInfo{
            const char* name;
            int priority;
            bool enabled;
            bool live;
            bool external;
            bool ownsPointer;
            int ownedButtons;
            int ownedKeys;
        };
        void getLayerInfo(std::vector<LayerInfo>* outInfo) const;

        static const int NUM_ROUTED_MOUSE_BUTTONS = 3;

    private:
        //Which engine system a layer represents. The engine's own layers are
        //dispatched through this rather than the callback struct, as there's no
        //library boundary between them and the router, and the gui layer needs
        //to report an intersection separately from whether it consumed.
        enum class BuiltinKind : uint8{
            External,
            SystemHotkeys,
            Gui,
            GameSink
        };

        struct Layer{
            InputLayerHandle handle;
            const char* name;
            int priority;
            BuiltinKind kind;
            bool enabled;
            InputLayerCallbacks cb;
            void* userData;
        };

        std::vector<Layer> mLayers; //Sorted by descending priority.
        InputLayerHandle mNextHandle;

        InputLayerHandle mMouseButtonOwner[NUM_ROUTED_MOUSE_BUTTONS];
        //Only contains scancodes which are currently held by a layer, so this
        //never has to be sized against the platform's scancode count.
        std::unordered_map<int, InputLayerHandle> mKeyOwner;
        InputLayerHandle mPointerFocus;

        //Kept so pointer focus can be recalculated without waiting for the user
        //to move the mouse again.
        float mLastMouseX, mLastMouseY;

        InputManager* mInputManager;
        GuiInputProcessor* mGuiInputProcessor;
        Window* mWindow;

        bool mExternalLayersEnabled;
        bool mGuiConsumes;

        Layer* _findLayer(InputLayerHandle handle);
        const Layer* _findLayer(InputLayerHandle handle) const;
        //Whether this layer should be offered new events at all.
        bool _layerAcceptsEvents(const Layer& l) const;
        bool _layerIsLive(const Layer& l) const;

        //Dispatch a single event to one layer, returning whether it consumed.
        bool _dispatchMouseButton(const Layer& l, int button, bool pressed);
        bool _dispatchKey(const Layer& l, int scancode, int keycode, int keyMod, bool pressed);
        bool _dispatchWheel(const Layer& l, float x, float y);
        //Engine level hotkeys, which sit above everything so a debug overlay
        //can't stop the engine's own tooling being toggled.
        bool _dispatchHotkey(int scancode, bool pressed);
        void _dispatchMouseMove(const Layer& l, float x, float y, float normX, float normY, bool focused);

        void _notifyCancelled(const Layer& l);
        void _notifyPointerFocusLost(const Layer& l);

        //Recalculate which layer owns the pointer and tell the loser.
        InputLayerHandle _resolvePointerFocus();
        InputLayerHandle _heldButtonOwner() const;
        void _clearOwnershipFor(InputLayerHandle handle);
        void _normalise(float x, float y, float* outX, float* outY) const;
    };
}
