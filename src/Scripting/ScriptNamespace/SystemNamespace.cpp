#include "SystemNamespace.h"

#include "System/Util/PathUtils.h"
#include "filesystem/path.h"

#include <rapidjson/filereadstream.h>
#include <rapidjson/error/en.h>

namespace AV{

    SQInteger SystemNamespace::makeDirectory(HSQUIRRELVM vm){
        const SQChar *path;
        sq_getstring(vm, 2, &path);

        std::string outString;
        formatResToPath(path, outString);
        //TODO make this only able to read and write in the saves directory.

        bool result = filesystem::create_directory(outString);
        if(!result) return sq_throwerror(vm, "Error creating directory.");

        return 0;
    }

    void SystemNamespace::_readJsonValue(HSQUIRRELVM vm, const rapidjson::Value& value){
        using namespace rapidjson;
        Type t = value.GetType();
        switch(t){
            case kNullType: sq_pushnull(vm); break;
            case kFalseType: sq_pushbool(vm, false); break;
            case kTrueType: sq_pushbool(vm, true); break;
            case kNumberType:{
                if(value.IsInt()){
                    sq_pushinteger(vm, value.GetInt());
                }
                else if(value.IsDouble()){
                    sq_pushfloat(vm, value.GetDouble());
                }
                break;
            }
            case kStringType:{
                sq_pushstring(vm, value.GetString(), value.GetStringLength());
                break;
            }
            case kObjectType:{
                sq_newtable(vm);
                for(Value::ConstMemberIterator memItr = value.MemberBegin(); memItr != value.MemberEnd(); ++memItr){
                    const GenericMember<UTF8<>, MemoryPoolAllocator<>>& member = *memItr;
                    _readJsonObject(vm, member);
                }
                break;
            }
            case kArrayType:{
                sq_newarray(vm, 0);
                int count = 0;
                for(Value::ConstValueIterator memItr = value.Begin(); memItr != value.End(); ++memItr){
                    _readJsonValue(vm, *memItr);
                    sq_arrayappend(vm, -2);
                    count++;
                }
                break;
            }
            default:{
                assert(false);
            }
        }
    }

    void SystemNamespace::_readJsonObject(HSQUIRRELVM vm, const rapidjson::GenericMember<rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<>>& value){
        using namespace rapidjson;

        sq_pushstring(vm, value.name.GetString(), value.name.GetStringLength());
        _readJsonValue(vm, value.value);
        sq_newslot(vm,-3,SQFalse);
    }

    SQInteger SystemNamespace::readJSONAsTable(HSQUIRRELVM vm){
        const SQChar *path;
        sq_getstring(vm, 2, &path);

        std::string outString;
        formatResToPath(path, outString);

        using namespace rapidjson;

        FILE* fp = fopen(outString.c_str(), "r");
        assert(fp);
        char readBuffer[65536];
        rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        rapidjson::Document d;
        d.ParseStream(is);
        fclose(fp);

        if(d.HasParseError()){
            char c[50];
            sprintf(c, "Error parsing file: %s", rapidjson::GetParseError_En(d.GetParseError()));
            return sq_throwerror(vm, &c[0]);
        }

        sq_newtable(vm);
        for(Value::ConstMemberIterator memItr = d.MemberBegin(); memItr != d.MemberEnd(); ++memItr){
            const GenericMember<UTF8<>, MemoryPoolAllocator<>>& member = *memItr;

            _readJsonObject(vm, member);
        }

        return 1;
    }

    /**SQNamespace
    @name _system
    @desc Functions relating to the underlying system.
    */
    void SystemNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name mkdir
        @desc Create a directory at the specified path.
        */
        ScriptUtils::addFunction(vm, makeDirectory, "mkdir", 2, ".s");
        /**SQFunction
        @name readJSONAsTable
        @desc Read a json file, returning the json data as a table object.
        */
        ScriptUtils::addFunction(vm, readJSONAsTable, "readJSONAsTable", 2, ".s");

    }
}
