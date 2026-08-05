#pragma once

#include <squirrel.h>
#include <sqstdaux.h>

#include <string>
#include <vector>

namespace AV{

    /**
    Copy squirrel values between two virtual machines.

    Two vms opened with separate sq_open calls do not share an SQSharedState, a string table or a
    garbage collector, so nothing but a by-value copy can legitimately cross between them. Only
    null, bool, integer, float, string, table and array are copyable. Everything else - closures,
    userdata, instances, classes, threads, weakrefs - is rejected by default, because the only
    alternative is to splice an object owned by one vm's reference graph into the other's, where
    the receiving collector will eventually free memory it does not own.

    A table referenced from two places in the same source graph is copied twice. Sharing cannot be
    reconstructed on the destination side without an identity map, and "values only" is the
    contract, so this is intended rather than a limitation to be fixed. Only true cycles are errors.

    Stack contract, for every method: on success srcvm is unchanged and dstvm has exactly one new
    value on top. On failure both stacks are restored to what they were on entry and getError()
    describes what went wrong.
    */
    class SquirrelDeepCopy{
    public:
        /**
        What to do with a value whose type cannot be copied between vms.
        */
        enum class UnsupportedPolicy{
            //Abandon the copy and record an error. The default.
            Reject,
            //Substitute null and keep going, for a caller that wants a lossy copy.
            PushNull
        };

        /**
        Copy the value at srcidx in srcvm onto the top of dstvm's stack.
        Negative indices are accepted and resolved against srcvm's current top.
        */
        bool deepCopyValue(HSQUIRRELVM srcvm, HSQUIRRELVM dstvm, SQInteger srcidx) const;
        bool deepCopyTable(HSQUIRRELVM srcvm, HSQUIRRELVM dstvm, SQInteger srcidx) const;
        bool deepCopyArray(HSQUIRRELVM srcvm, HSQUIRRELVM dstvm, SQInteger srcidx) const;

        bool hasError() const { return !mError.empty(); }
        /**
        A description of the first failure, naming the offending type and where in the value it
        was found, i.e. "cannot copy value of type 'closure' at segments[3].onPaint".
        */
        const std::string& getError() const { return mError; }

        UnsupportedPolicy mUnsupportedPolicy = UnsupportedPolicy::Reject;
        //Maximum container nesting. Guards against a graph deep enough to exhaust the c stack.
        size_t mMaxDepth = 64;

        /**
        Copy a single value, reporting the failure reason through outError.
        Convenience for the common case of one copy per call site.
        */
        static bool copyValueStrict(HSQUIRRELVM srcvm, HSQUIRRELVM dstvm, SQInteger srcidx, std::string& outError);

    private:
        //The methods are const for backwards compatibility, so the working state has to be mutable.
        mutable std::string mError;
        //Raw pointers of the containers on the current copy path, for cycle detection.
        mutable std::vector<SQRawObjectVal> mVisiting;
        //Preformatted path segments, i.e. ".name" or "[3]", for the error message.
        mutable std::vector<std::string> mPath;

        void _begin() const;
        bool _copyValue(HSQUIRRELVM srcvm, HSQUIRRELVM dstvm, SQInteger srcidx) const;
        bool _copyTable(HSQUIRRELVM srcvm, HSQUIRRELVM dstvm, SQInteger srcidx) const;
        bool _copyArray(HSQUIRRELVM srcvm, HSQUIRRELVM dstvm, SQInteger srcidx) const;

        //Returns false so failure paths can 'return _fail(...)'.
        bool _fail(const std::string& message) const;
        std::string _pathString() const;
        //The path segment describing the table key at keyIdx in srcvm, i.e. ".name" or "[3]".
        std::string _pathSegmentForKey(HSQUIRRELVM srcvm, SQInteger keyIdx) const;

        static SQInteger _absIndex(HSQUIRRELVM vm, SQInteger idx);
        static const char* _typeName(SQObjectType type);
    };

}
