#ifdef FLIGHT_RECORDER

#pragma once

#include <vector>
#include <cstdint>

namespace AV{

    enum class ScriptEventType : uint8_t{
        Call,
        Return
    };

    /**
    One squirrel call or return, as the debug hook reported it.
    Deliberately small and trivially copyable: one of these is written on every call the vm
    makes, so anything expensive here is paid thousands of times a second.
    */
    struct ScriptEvent{
        uint64_t timeNs = 0;
        uint32_t funcIndex = 0;
        int32_t line = 0;
        ScriptEventType type = ScriptEventType::Call;
    };

    /**
    Circular buffer of script events, plus the shadow call stack needed to make sense of them.

    Events are addressed by a monotonic absolute index that never wraps, so a frame can record
    the range of events belonging to it and still tell, later, whether the ring has since
    overwritten them (see available()). The ring itself only holds the most recent capacity()
    events.

    Contains no squirrel state - the caller resolves functions to indices - so it is unit
    testable without a vm.
    */
    class ScriptEventRing{
    public:
        ScriptEventRing() = default;

        /**
        Set the ring size and drop everything held. Also resets the absolute index.
        */
        void configure(uint32_t capacity);

        /**
        Record a call. Pushes funcIndex onto the shadow stack and tracks the deepest stack
        reached since the last takeDeepestStack().
        */
        void pushCall(uint32_t funcIndex, int32_t line, uint64_t timeNs);

        /**
        Record a return, popping the shadow stack.

        A return with an empty stack is recorded but pops nothing: collection can begin part
        way through a call, in which case the matching call was never seen.
        */
        void pushReturn(int32_t line, uint64_t timeNs);

        /**
        The absolute index the next event will be given. A frame's slice is the value of this
        at the start of the frame through its value at the end.
        */
        uint64_t nextIndex() const { return mNextIndex; }

        /**
        The oldest absolute index still held. Anything below this has been overwritten.
        */
        uint64_t oldestIndex() const;

        /**
        Whether the whole half open range [begin, end) is still in the ring.
        */
        bool available(uint64_t begin, uint64_t end) const;

        /**
        Read one event by absolute index. Only valid when available(index, index+1).
        */
        const ScriptEvent& at(uint64_t absoluteIndex) const;

        /**
        Copy out the events in [begin, end), clamped to what is still held.
        */
        std::vector<ScriptEvent> slice(uint64_t begin, uint64_t end) const;

        /**
        The deepest call stack seen since the last call to this function, as function indices
        outermost first, and reset the high water mark to the current stack.

        This is what gives an F3 capture a representative stack shape for each frame: by the
        time a keypress is handled the vm is idle and its real stack is empty.
        */
        std::vector<uint32_t> takeDeepestStack();

        //Current live shadow stack depth.
        size_t stackDepth() const { return mStack.size(); }
        //Number of events currently held.
        size_t size() const;
        uint32_t capacity() const { return mCapacity; }
        //Total events ever recorded, including those since overwritten.
        uint64_t totalEvents() const { return mNextIndex; }

        void clear();

    private:
        std::vector<ScriptEvent> mEvents;
        uint32_t mCapacity = 0;
        uint64_t mNextIndex = 0;

        //Function indices of the calls currently open.
        std::vector<uint32_t> mStack;
        //Deepest mStack seen since the last takeDeepestStack().
        std::vector<uint32_t> mDeepest;
    };
}

#endif
