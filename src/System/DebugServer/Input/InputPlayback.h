#ifdef DEBUG_SERVER

#pragma once

#include "Input/InputPrerequisites.h"

#include <string>
#include <vector>
#include <cstdint>

namespace AV{
    /**
    Injects spoofed input into the engine on behalf of the debug server, with frame
    lifetimes so a "press for 10 frames" request behaves like a real button hold.

    Timing model — set once, not re-assert. The engine reads input in the fixed-step
    loop before InputManager::update() increments each action's duration, and
    INPUT_TYPE_PRESSED is defined as "duration == 0". Re-asserting a held button every
    frame would reset its duration and refire "pressed" continuously. So this class
    presses once when a spoof begins and releases once when it expires, exactly like the
    hardware path (SDL2Window::_handleKey fires once per physical press/release).

    Main-thread only: apply* is called from queued closures during the pump, and update()
    from the pump itself. Both run on the main thread, so no locking is needed here.
    */
    class InputPlayback{
    public:
        struct Result{
            bool ok = false;
            std::string error;
            //Frame the input auto-releases at; -1 when held indefinitely or applied
            //instantaneously (mouse move).
            int64_t releasesAtFrame = -1;
        };

        /**
        Press or release a button action. pressed=false releases immediately and drops
        any tracked hold for the same action. frames < 0 holds until explicitly released.
        */
        Result applyButtonAction(const std::string& actionName, bool pressed, int frames);

        /**
        Set a stick/axis action to (x, y). frames < 0 holds until reset. Passing (0,0)
        recentres and drops the tracked hold.
        */
        Result applyAxisAction(const std::string& actionName, float x, float y, int frames);

        /**
        Press or release a mouse button (0 left, 1 right, 2 middle), routed through the
        GUI so hit-testing matches a real click.
        */
        Result applyMouseButton(int button, bool pressed, int frames);

        /**
        Warp the pointer to a normalised (0-1) window position. Instantaneous, no lifetime.
        */
        Result applyMouseMove(float normX, float normY);

        /**
        Release everything currently spoofed.
        */
        void clear();

        /**
        Tick lifetimes once per rendered frame; release entries whose frames elapse.
        */
        void update();

        uint64_t getFrameNumber() const { return mFrameNumber; }

        struct ActiveEntry{
            std::string description;
            int framesRemaining; //-1 for indefinite holds.
        };
        std::vector<ActiveEntry> getActive() const;

    private:
        enum class Kind{ ButtonAction, AxisAction, MouseButton };
        struct Spoof{
            Kind kind;
            ActionHandle handle = 0;
            int button = 0;          //Mouse button index.
            std::string description;
            int framesRemaining;     //-1 for indefinite.
        };

        void _release(const Spoof& spoof);
        //Remove and release any tracked entry matching kind + id.
        void _dropMatching(Kind kind, ActionHandle handle, int button);

        std::vector<Spoof> mActive;
        uint64_t mFrameNumber = 0;

        //Spoofed controller device. Setting device 0 also populates the ANY device
        //aggregate, so games querying either see the input.
        static const InputDeviceId SPOOF_DEVICE = 0;
    };
}

#endif
