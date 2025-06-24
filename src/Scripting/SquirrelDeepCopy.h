#include <squirrel.h>
#include <sqstdaux.h>

namespace AV{

    class SquirrelDeepCopy{
    public:
        bool deepCopyValue(HSQUIRRELVM srcvm, HSQUIRRELVM dstvm, SQInteger srcidx) const;
        bool deepCopyTable(HSQUIRRELVM srcvm, HSQUIRRELVM dstvm, SQInteger srcidx) const;
        bool deepCopyArray(HSQUIRRELVM srcvm, HSQUIRRELVM dstvm, SQInteger srcidx) const;
    };

}