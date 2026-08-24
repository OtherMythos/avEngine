#include "gtest/gtest.h"

#include "Input/InputRouter.h"
#include "Input/InputManager.h"

#include <vector>

using namespace AV;

namespace {

    /**
    A stand in for a plugin's layer.

    The callback struct takes raw function pointers, so each instance is reached
    through the userData rather than a bound method.
    */
    struct FakeLayer{
        bool live = true;
        bool wantPointer = false;
        bool wantKeyboard = false;
        bool wantText = false;

        //What this layer claims when offered an event.
        bool consumeButtons = false;
        bool consumeKeys = false;
        bool consumeWheel = false;
        bool consumeText = false;

        struct ButtonEvent{ int button; bool pressed; };
        std::vector<ButtonEvent> buttons;
        struct KeyEvent{ int scancode; bool pressed; };
        std::vector<KeyEvent> keys;
        int moves = 0;
        int focusedMoves = 0;
        int cancelled = 0;
        int focusLost = 0;

        static bool isLive(void* u){ return static_cast<FakeLayer*>(u)->live; }
        static bool wantsPointer(void* u){ return static_cast<FakeLayer*>(u)->wantPointer; }
        static bool wantsKeyboard(void* u){ return static_cast<FakeLayer*>(u)->wantKeyboard; }
        static bool wantsTextInput(void* u){ return static_cast<FakeLayer*>(u)->wantText; }

        static void onMouseMove(float, float, float, float, bool focused, void* u){
            FakeLayer* l = static_cast<FakeLayer*>(u);
            l->moves++;
            if(focused) l->focusedMoves++;
        }
        static bool onMouseButton(int button, bool pressed, void* u){
            FakeLayer* l = static_cast<FakeLayer*>(u);
            l->buttons.push_back({button, pressed});
            return l->consumeButtons;
        }
        static bool onMouseWheel(float, float, void* u){
            return static_cast<FakeLayer*>(u)->consumeWheel;
        }
        static bool onKey(int scancode, int, int, bool pressed, void* u){
            FakeLayer* l = static_cast<FakeLayer*>(u);
            l->keys.push_back({scancode, pressed});
            return l->consumeKeys;
        }
        static bool onTextInput(const char*, void* u){
            return static_cast<FakeLayer*>(u)->consumeText;
        }
        static void onInputCancelled(void* u){ static_cast<FakeLayer*>(u)->cancelled++; }
        static void onPointerFocusLost(void* u){ static_cast<FakeLayer*>(u)->focusLost++; }

        static InputLayerCallbacks callbacks(){
            InputLayerCallbacks cb;
            memset(&cb, 0, sizeof(InputLayerCallbacks));
            cb.isLive = &FakeLayer::isLive;
            cb.wantsPointer = &FakeLayer::wantsPointer;
            cb.wantsKeyboard = &FakeLayer::wantsKeyboard;
            cb.wantsTextInput = &FakeLayer::wantsTextInput;
            cb.onMouseMove = &FakeLayer::onMouseMove;
            cb.onMouseButton = &FakeLayer::onMouseButton;
            cb.onMouseWheel = &FakeLayer::onMouseWheel;
            cb.onKey = &FakeLayer::onKey;
            cb.onTextInput = &FakeLayer::onTextInput;
            cb.onInputCancelled = &FakeLayer::onInputCancelled;
            cb.onPointerFocusLost = &FakeLayer::onPointerFocusLost;
            return cb;
        }
    };

    class InputRouterTests : public ::testing::Test{
    protected:
        InputManager inMan;
        InputRouter router;

        virtual void SetUp(){
            //A null gui processor and window leave the gui layer transparent,
            //which is what lets the router be tested without Colibri or a window.
            router.initialise(&inMan, 0, 0);
        }

        InputLayerHandle addOverlay(FakeLayer* layer, int priority = INPUT_PRIORITY_OVERLAY){
            return router.addLayer("test", priority, FakeLayer::callbacks(), layer);
        }
    };

    TEST_F(InputRouterTests, unconsumedPressReachesTheGame){
        FakeLayer overlay;
        addOverlay(&overlay);

        router.injectMouseButton(0, true);

        EXPECT_TRUE(inMan.getMouseButton(0));
        EXPECT_TRUE(inMan.getMousePressed(0));
        //The layer was still offered it, it simply declined.
        ASSERT_EQ(1u, overlay.buttons.size());
        EXPECT_TRUE(overlay.buttons[0].pressed);
    }

    TEST_F(InputRouterTests, consumedPressNeverReachesTheGame){
        FakeLayer overlay;
        overlay.consumeButtons = true;
        addOverlay(&overlay);

        router.injectMouseButton(0, true);

        EXPECT_FALSE(inMan.getMouseButton(0));
        EXPECT_FALSE(inMan.getMousePressed(0));
        //The press landing on ui is still reported, which is what the touch
        //events carry through to script.
        EXPECT_TRUE(inMan.getMouseGuiIntersected());
    }

    TEST_F(InputRouterTests, ownedReleaseGoesToTheOwnerAloneAndNotTheGame){
        FakeLayer overlay;
        overlay.consumeButtons = true;
        addOverlay(&overlay);

        router.injectMouseButton(0, true);
        router.injectMouseButton(0, false);

        ASSERT_EQ(2u, overlay.buttons.size());
        EXPECT_TRUE(overlay.buttons[0].pressed);
        EXPECT_FALSE(overlay.buttons[1].pressed);
        //The game never saw either half, so nothing is left half pressed.
        EXPECT_FALSE(inMan.getMouseButton(0));
        EXPECT_FALSE(inMan.getMouseReleased(0));
    }

    TEST_F(InputRouterTests, unownedReleaseIsBroadcastAndNeverConsumed){
        FakeLayer overlay;
        addOverlay(&overlay);

        //Press falls through, so the game holds the button.
        router.injectMouseButton(0, true);
        EXPECT_TRUE(inMan.getMouseButton(0));

        //The layer now claims events, but a release it does not own must not be
        //swallowed, or the game would hold that button forever.
        overlay.consumeButtons = true;
        router.injectMouseButton(0, false);

        EXPECT_FALSE(inMan.getMouseButton(0));
        EXPECT_TRUE(inMan.getMouseReleased(0));
    }

    TEST_F(InputRouterTests, aLayerGoingQuietMidDragStillGetsItsRelease){
        FakeLayer overlay;
        overlay.consumeButtons = true;
        addOverlay(&overlay);

        router.injectMouseButton(0, true);
        //The overlay stops drawing, so it stops being offered new events.
        overlay.live = false;
        router.injectMouseButton(0, false);

        ASSERT_EQ(2u, overlay.buttons.size());
        EXPECT_FALSE(overlay.buttons[1].pressed);
        //And the release still did not leak into the game.
        EXPECT_FALSE(inMan.getMouseButton(0));
    }

    TEST_F(InputRouterTests, aNonLiveLayerIsTransparentToNewEvents){
        FakeLayer overlay;
        overlay.consumeButtons = true;
        overlay.live = false;
        addOverlay(&overlay);

        router.injectMouseButton(0, true);

        EXPECT_TRUE(overlay.buttons.empty());
        EXPECT_TRUE(inMan.getMouseButton(0));
    }

    TEST_F(InputRouterTests, keyHeldBeforeCaptureStillReleasesIntoTheGame){
        FakeLayer overlay;
        addOverlay(&overlay);

        const int scancodeW = 26;
        router.injectKey(scancodeW, 'w', 0, true);
        EXPECT_TRUE(inMan.getKeyboardInput(scancodeW));

        //A text field takes focus while the key is still down.
        overlay.consumeKeys = true;
        router.injectKey(scancodeW, 'w', 0, false);

        //The release pairs with the press the game actually saw, so nothing sticks.
        EXPECT_FALSE(inMan.getKeyboardInput(scancodeW));
    }

    TEST_F(InputRouterTests, consumedKeyNeverReachesTheGame){
        FakeLayer overlay;
        overlay.consumeKeys = true;
        addOverlay(&overlay);

        const int scancodeW = 26;
        router.injectKey(scancodeW, 'w', 0, true);

        EXPECT_FALSE(inMan.getKeyboardInput(scancodeW));
        router.injectKey(scancodeW, 'w', 0, false);
        EXPECT_FALSE(inMan.getKeyboardInput(scancodeW));
    }

    TEST_F(InputRouterTests, motionIsNeverConsumed){
        FakeLayer overlay;
        overlay.wantPointer = true;
        overlay.consumeButtons = true;
        addOverlay(&overlay);

        router.injectMouseMove(40.0f, 90.0f);

        //Position reaches the game whoever owns the pointer, so nothing
        //downstream ever sees the cursor freeze.
        EXPECT_EQ(40, inMan.getMouseX());
        EXPECT_EQ(90, inMan.getMouseY());
        EXPECT_EQ(1, overlay.moves);
        EXPECT_EQ(1, overlay.focusedMoves);
    }

    TEST_F(InputRouterTests, pointerFocusIsHeldForTheWholeDrag){
        FakeLayer lower;
        lower.consumeButtons = true;
        InputLayerHandle lowerHandle = addOverlay(&lower, INPUT_PRIORITY_OVERLAY);

        FakeLayer upper;
        InputLayerHandle upperHandle = addOverlay(&upper, INPUT_PRIORITY_OVERLAY + 10);
        EXPECT_NE(lowerHandle, upperHandle);

        //The lower layer takes a button, starting a drag.
        router.injectMouseButton(0, true);

        //A higher layer now wants the pointer, but the drag must keep it.
        upper.wantPointer = true;
        router.injectMouseMove(10.0f, 10.0f);

        EXPECT_EQ(1, lower.focusedMoves);
        EXPECT_EQ(0, upper.focusedMoves);

        //Once the drag ends the higher layer takes over.
        router.injectMouseButton(0, false);
        router.injectMouseMove(20.0f, 20.0f);
        EXPECT_EQ(1, upper.focusedMoves);
        EXPECT_EQ(1, lower.focusLost);
    }

    TEST_F(InputRouterTests, higherPriorityLayerWinsThePress){
        FakeLayer lower;
        lower.consumeButtons = true;
        addOverlay(&lower, INPUT_PRIORITY_OVERLAY);

        FakeLayer upper;
        upper.consumeButtons = true;
        addOverlay(&upper, INPUT_PRIORITY_OVERLAY + 10);

        router.injectMouseButton(0, true);

        EXPECT_EQ(1u, upper.buttons.size());
        //The lower layer is never even offered it.
        EXPECT_TRUE(lower.buttons.empty());
    }

    TEST_F(InputRouterTests, removingAnOwningLayerLeaksNothingDownward){
        FakeLayer overlay;
        overlay.consumeButtons = true;
        InputLayerHandle handle = addOverlay(&overlay);

        router.injectMouseButton(0, true);
        router.removeLayer(handle);

        EXPECT_EQ(1, overlay.cancelled);
        //The game never saw the press, so it must not now see a lone release.
        EXPECT_FALSE(inMan.getMouseButton(0));
        EXPECT_FALSE(inMan.getMouseReleased(0));

        //And the button is free again for whoever is left.
        router.injectMouseButton(0, true);
        EXPECT_TRUE(inMan.getMouseButton(0));
    }

    TEST_F(InputRouterTests, disablingALayerRevokesWhatItHeld){
        FakeLayer overlay;
        overlay.consumeButtons = true;
        InputLayerHandle handle = addOverlay(&overlay);

        router.injectMouseButton(0, true);
        router.setLayerEnabled(handle, false);
        EXPECT_EQ(1, overlay.cancelled);

        //A press now falls through to the game.
        router.injectMouseButton(0, true);
        EXPECT_TRUE(inMan.getMouseButton(0));
        EXPECT_EQ(1u, overlay.buttons.size()); //Still just the original press.
    }

    TEST_F(InputRouterTests, externalLayersCanBeDisabledWholesale){
        FakeLayer overlay;
        overlay.consumeButtons = true;
        addOverlay(&overlay);

        router.setExternalLayersEnabled(false);
        router.injectMouseButton(0, true);

        EXPECT_TRUE(overlay.buttons.empty());
        EXPECT_TRUE(inMan.getMouseButton(0));
    }

    TEST_F(InputRouterTests, cancelAllInputClearsEverythingAndIsIdempotent){
        FakeLayer overlay;
        overlay.consumeButtons = true;
        overlay.consumeKeys = true;
        addOverlay(&overlay);

        router.injectMouseButton(0, true);
        router.injectKey(26, 'w', 0, true);

        router.cancelAllInput();
        EXPECT_EQ(1, overlay.cancelled);
        //The owner was told its button came up.
        ASSERT_EQ(2u, overlay.buttons.size());
        EXPECT_FALSE(overlay.buttons[1].pressed);

        router.cancelAllInput();
        //Nothing further to release, so no new button events.
        EXPECT_EQ(2u, overlay.buttons.size());

        //Ownership is genuinely clear.
        overlay.consumeButtons = false;
        router.injectMouseButton(0, true);
        EXPECT_TRUE(inMan.getMouseButton(0));
    }

    TEST_F(InputRouterTests, cancelAllInputReleasesHeldKeysInTheGame){
        const int scancodeW = 26;
        router.injectKey(scancodeW, 'w', 0, true);
        EXPECT_TRUE(inMan.getKeyboardInput(scancodeW));

        //This is what stops a key sticking down when the window loses focus.
        router.cancelAllInput();
        EXPECT_FALSE(inMan.getKeyboardInput(scancodeW));
    }

    TEST_F(InputRouterTests, textInputIsRequestedByAnyLayerThatWantsIt){
        FakeLayer overlay;
        addOverlay(&overlay);

        EXPECT_FALSE(router.shouldTextInputEnable());
        overlay.wantText = true;
        EXPECT_TRUE(router.shouldTextInputEnable());

        //A layer which is not live cannot ask for text input.
        overlay.live = false;
        EXPECT_FALSE(router.shouldTextInputEnable());
    }

    TEST_F(InputRouterTests, equalPrioritiesKeepRegistrationOrder){
        FakeLayer first;
        first.consumeButtons = true;
        addOverlay(&first);

        FakeLayer second;
        second.consumeButtons = true;
        addOverlay(&second);

        router.injectMouseButton(0, true);

        EXPECT_EQ(1u, first.buttons.size());
        EXPECT_TRUE(second.buttons.empty());
    }

#ifdef DEBUGGING_TOOLS
    TEST_F(InputRouterTests, engineHotkeysSitAboveEverything){
        FakeLayer overlay;
        overlay.consumeKeys = true;
        addOverlay(&overlay);

        //F1 toggles the engine's own stats display. It has to win even against a
        //debug overlay which is swallowing every other key, or the tooling used
        //to diagnose that overlay could be disabled by it.
        router.injectKey(INPUT_SCANCODE_F1, 0, 0, true);

        EXPECT_TRUE(overlay.keys.empty());
        EXPECT_FALSE(inMan.getKeyboardInput(INPUT_SCANCODE_F1));

        //An ordinary key is unaffected and still reaches the overlay.
        router.injectKey(26, 'w', 0, true);
        ASSERT_EQ(1u, overlay.keys.size());
        EXPECT_EQ(26, overlay.keys[0].scancode);
    }
#endif

    TEST_F(InputRouterTests, eachButtonIsOwnedIndependently){
        FakeLayer overlay;
        addOverlay(&overlay);

        //Left falls through to the game.
        router.injectMouseButton(0, true);
        //Right is taken by the overlay.
        overlay.consumeButtons = true;
        router.injectMouseButton(1, true);

        EXPECT_TRUE(inMan.getMouseButton(0));
        EXPECT_FALSE(inMan.getMouseButton(1));

        router.injectMouseButton(0, false);
        router.injectMouseButton(1, false);
        EXPECT_FALSE(inMan.getMouseButton(0));
        EXPECT_FALSE(inMan.getMouseButton(1));
    }
}
