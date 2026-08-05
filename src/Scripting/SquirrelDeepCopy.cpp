#include "SquirrelDeepCopy.h"

#include <algorithm>

namespace AV{

    //A guard which restores both stacks unless it is released. Every failure path in this file
    //goes through one, which is what makes the stack contract in the header hold uniformly
    //rather than depending on each error branch popping the right number of values.
    namespace{
        struct StackGuard{
            StackGuard(HSQUIRRELVM src, HSQUIRRELVM dst)
                : mSrc(src), mDst(dst), mSrcTop(sq_gettop(src)), mDstTop(sq_gettop(dst)){

            }
            ~StackGuard(){
                if(mReleased) return;
                sq_settop(mSrc, mSrcTop);
                sq_settop(mDst, mDstTop);
            }
            void release() { mReleased = true; }

            HSQUIRRELVM mSrc, mDst;
            SQInteger mSrcTop, mDstTop;
            bool mReleased = false;
        };

        struct PathScope{
            PathScope(std::vector<std::string>& path, const std::string& segment)
                : mPath(path){
                mPath.push_back(segment);
            }
            ~PathScope() { mPath.pop_back(); }
            std::vector<std::string>& mPath;
        };

        struct VisitScope{
            VisitScope(std::vector<SQRawObjectVal>& visiting, SQRawObjectVal val)
                : mVisiting(visiting){
                mVisiting.push_back(val);
            }
            ~VisitScope() { mVisiting.pop_back(); }
            std::vector<SQRawObjectVal>& mVisiting;
        };
    }

    const char* SquirrelDeepCopy::_typeName(SQObjectType type){
        switch(type){
            case OT_NULL: return "null";
            case OT_INTEGER: return "integer";
            case OT_FLOAT: return "float";
            case OT_BOOL: return "bool";
            case OT_STRING: return "string";
            case OT_TABLE: return "table";
            case OT_ARRAY: return "array";
            case OT_USERDATA: return "userdata";
            case OT_CLOSURE: return "closure";
            case OT_NATIVECLOSURE: return "native closure";
            case OT_GENERATOR: return "generator";
            case OT_USERPOINTER: return "userpointer";
            case OT_THREAD: return "thread";
            case OT_FUNCPROTO: return "funcproto";
            case OT_CLASS: return "class";
            case OT_INSTANCE: return "instance";
            case OT_WEAKREF: return "weakref";
            case OT_OUTER: return "outer";
            default: return "unknown";
        }
    }

    SQInteger SquirrelDeepCopy::_absIndex(HSQUIRRELVM vm, SQInteger idx){
        return idx < 0 ? sq_gettop(vm) + idx + 1 : idx;
    }

    void SquirrelDeepCopy::_begin() const{
        mError.clear();
        mVisiting.clear();
        mPath.clear();
    }

    std::string SquirrelDeepCopy::_pathString() const{
        if(mPath.empty()) return "<root>";

        std::string out;
        for(const std::string& s : mPath) out += s;
        //The first segment of a table path carries a leading dot which reads badly at the start.
        if(!out.empty() && out[0] == '.') out.erase(0, 1);
        return out;
    }

    bool SquirrelDeepCopy::_fail(const std::string& message) const{
        //The first error is the deepest and most specific, so later ones recorded while the
        //recursion unwinds must not overwrite it.
        if(mError.empty()) mError = message + " at " + _pathString();
        return false;
    }

    std::string SquirrelDeepCopy::_pathSegmentForKey(HSQUIRRELVM srcvm, SQInteger keyIdx) const{
        switch(sq_gettype(srcvm, keyIdx)){
            case OT_STRING: {
                const SQChar* key = 0;
                if(SQ_SUCCEEDED(sq_getstring(srcvm, keyIdx, &key)) && key){
                    return std::string(".") + key;
                }
                break;
            }
            case OT_INTEGER: {
                SQInteger key = 0;
                if(SQ_SUCCEEDED(sq_getinteger(srcvm, keyIdx, &key))){
                    return "[" + std::to_string(static_cast<long long>(key)) + "]";
                }
                break;
            }
            default: break;
        }
        return "[?]";
    }

    bool SquirrelDeepCopy::copyValueStrict(HSQUIRRELVM srcvm, HSQUIRRELVM dstvm, SQInteger srcidx, std::string& outError){
        SquirrelDeepCopy copy;
        if(copy.deepCopyValue(srcvm, dstvm, srcidx)) return true;

        outError = copy.getError();
        if(outError.empty()) outError = "failed to copy value between script vms";
        return false;
    }

    bool SquirrelDeepCopy::deepCopyValue(HSQUIRRELVM srcvm, HSQUIRRELVM dstvm, SQInteger srcidx) const{
        _begin();
        return _copyValue(srcvm, dstvm, _absIndex(srcvm, srcidx));
    }

    bool SquirrelDeepCopy::deepCopyTable(HSQUIRRELVM srcvm, HSQUIRRELVM dstvm, SQInteger srcidx) const{
        _begin();
        return _copyTable(srcvm, dstvm, _absIndex(srcvm, srcidx));
    }

    bool SquirrelDeepCopy::deepCopyArray(HSQUIRRELVM srcvm, HSQUIRRELVM dstvm, SQInteger srcidx) const{
        _begin();
        return _copyArray(srcvm, dstvm, _absIndex(srcvm, srcidx));
    }

    bool SquirrelDeepCopy::_copyValue(HSQUIRRELVM srcvm, HSQUIRRELVM dstvm, SQInteger srcidx) const{
        StackGuard guard(srcvm, dstvm);

        const SQObjectType type = sq_gettype(srcvm, srcidx);
        switch(type){
            case OT_NULL:
                sq_pushnull(dstvm);
                break;

            case OT_BOOL: {
                SQBool value;
                if(SQ_FAILED(sq_getbool(srcvm, srcidx, &value))) return _fail("could not read bool");
                sq_pushbool(dstvm, value);
                break;
            }

            case OT_INTEGER: {
                SQInteger value;
                if(SQ_FAILED(sq_getinteger(srcvm, srcidx, &value))) return _fail("could not read integer");
                sq_pushinteger(dstvm, value);
                break;
            }

            case OT_FLOAT: {
                SQFloat value;
                if(SQ_FAILED(sq_getfloat(srcvm, srcidx, &value))) return _fail("could not read float");
                sq_pushfloat(dstvm, value);
                break;
            }

            case OT_STRING: {
                const SQChar* value;
                if(SQ_FAILED(sq_getstring(srcvm, srcidx, &value))) return _fail("could not read string");
                //Copied by value into the destination vm's own string table.
                sq_pushstring(dstvm, value, -1);
                break;
            }

            case OT_TABLE:
                if(!_copyTable(srcvm, dstvm, srcidx)) return false;
                break;

            case OT_ARRAY:
                if(!_copyArray(srcvm, dstvm, srcidx)) return false;
                break;

            default:
                //Everything else would have to be shared rather than copied. See the header.
                if(mUnsupportedPolicy == UnsupportedPolicy::PushNull){
                    sq_pushnull(dstvm);
                    break;
                }
                return _fail(std::string("cannot copy value of type '") + _typeName(type) + "'");
        }

        guard.release();
        return true;
    }

    bool SquirrelDeepCopy::_copyTable(HSQUIRRELVM srcvm, HSQUIRRELVM dstvm, SQInteger srcidx) const{
        StackGuard guard(srcvm, dstvm);

        if(mVisiting.size() >= mMaxDepth) return _fail("value nested deeper than the copy depth limit");

        HSQOBJECT obj;
        sq_resetobject(&obj);
        if(SQ_FAILED(sq_getstackobj(srcvm, srcidx, &obj))) return _fail("could not read table");
        if(std::find(mVisiting.begin(), mVisiting.end(), obj._unVal.raw) != mVisiting.end()){
            return _fail("cyclic reference");
        }
        VisitScope visit(mVisiting, obj._unVal.raw);

        sq_newtable(dstvm);

        sq_pushnull(srcvm); //The null iterator.
        while(SQ_SUCCEEDED(sq_next(srcvm, srcidx))){
            //srcvm now holds the key at -2 and the value at -1. Both are resolved to absolute
            //indices because copying the key pushes onto dstvm, not srcvm, but the value copy
            //recurses and it is easier to reason about with fixed indices.
            const SQInteger keyIdx = sq_gettop(srcvm) - 1;
            const SQInteger valIdx = sq_gettop(srcvm);

            //A key which is not copyable is reported against the container itself, since there
            //is no name yet to describe the slot by.
            if(!_copyValue(srcvm, dstvm, keyIdx)) return false;

            {
                PathScope scope(mPath, _pathSegmentForKey(srcvm, keyIdx));
                if(!_copyValue(srcvm, dstvm, valIdx)) return false;
            }

            //dstvm holds the new table, then the key, then the value.
            if(SQ_FAILED(sq_newslot(dstvm, -3, false))) return _fail("could not insert into copied table");

            sq_pop(srcvm, 2); //Key and value, ready for the next iteration.
        }
        sq_pop(srcvm, 1); //The null iterator.

        guard.release();
        return true;
    }

    bool SquirrelDeepCopy::_copyArray(HSQUIRRELVM srcvm, HSQUIRRELVM dstvm, SQInteger srcidx) const{
        StackGuard guard(srcvm, dstvm);

        if(mVisiting.size() >= mMaxDepth) return _fail("value nested deeper than the copy depth limit");

        HSQOBJECT obj;
        sq_resetobject(&obj);
        if(SQ_FAILED(sq_getstackobj(srcvm, srcidx, &obj))) return _fail("could not read array");
        if(std::find(mVisiting.begin(), mVisiting.end(), obj._unVal.raw) != mVisiting.end()){
            return _fail("cyclic reference");
        }
        VisitScope visit(mVisiting, obj._unVal.raw);

        const SQInteger size = sq_getsize(srcvm, srcidx);
        if(size < 0) return _fail("could not read array size");

        sq_newarray(dstvm, 0);

        for(SQInteger i = 0; i < size; i++){
            sq_pushinteger(srcvm, i);
            if(SQ_FAILED(sq_get(srcvm, srcidx))) return _fail("could not read array element");

            PathScope scope(mPath, "[" + std::to_string(static_cast<long long>(i)) + "]");
            if(!_copyValue(srcvm, dstvm, sq_gettop(srcvm))) return false;

            //Appending rather than setting by index keeps the destination array consistent with
            //what has actually been copied so far, which is what the guard restores on failure.
            if(SQ_FAILED(sq_arrayappend(dstvm, -2))) return _fail("could not append to copied array");

            sq_pop(srcvm, 1); //The element.
        }

        guard.release();
        return true;
    }

}
