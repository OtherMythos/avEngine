#include "SystemNamespace.h"

#include "System/Util/PathUtils.h"
#include "filesystem/path.h"

#include <rapidjson/filereadstream.h>
#include <rapidjson/error/en.h>
#include "rapidjson/filewritestream.h"
#include <rapidjson/writer.h>
#include "rapidjson/prettywriter.h"

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

    void SystemNamespace::_writeJSONProcessValue(HSQUIRRELVM vm, rapidjson::Value& key, rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator, bool isArray){
        SQObjectType objectType = sq_gettype(vm, -1);
        switch(objectType){
            case OT_STRING:{
                const SQChar* foundString;
                sq_getstring(vm, -1, &foundString);
                rapidjson::Value foundValue((char*)foundString, allocator);
                if(isArray) value.PushBack(foundValue, allocator);
                else value.AddMember(key, foundValue, allocator);
                break;
            }
            case OT_INTEGER:{
                SQInteger val;
                sq_getinteger(vm, -1, &val);
                if(isArray) value.PushBack(static_cast<int>(val), allocator);
                else value.AddMember(key, static_cast<int>(val), allocator);
                break;
            }
            case OT_FLOAT:{
                SQFloat val;
                sq_getfloat(vm, -1, &val);
                if(isArray) value.PushBack(static_cast<float>(val), allocator);
                else value.AddMember(key, static_cast<float>(val), allocator);
                break;
            }
            case OT_BOOL:{
                SQBool val;
                sq_getbool(vm, -1, &val);
                bool boolVal = val == SQTrue ? true : false;
                if(isArray) value.PushBack(boolVal, allocator);
                else value.AddMember(key, boolVal, allocator);
                break;
            }
            case OT_ARRAY:{
                rapidjson::Value arrayObject(rapidjson::kArrayType);
                sq_pushnull(vm);
                while(SQ_SUCCEEDED(sq_next(vm, -2))){
                    rapidjson::Value empty;
                    _writeJSONProcessValue(vm, empty, arrayObject, allocator, true);
                    sq_pop(vm, 2);
                }
                if(isArray) value.PushBack(arrayObject, allocator);
                else value.AddMember(key, arrayObject, allocator);
                sq_pop(vm, 1);
                break;
            }
            case OT_TABLE:{
                rapidjson::Value objVal(rapidjson::kObjectType);
                _writeJSONReadValuesFromTable(vm, objVal, allocator);
                if(isArray) value.AddMember(key, objVal, allocator);
                else value.AddMember(key, objVal, allocator);
                break;
            }
            default:{
                //Add null.
                rapidjson::Value val;
                if(isArray) value.PushBack(val, allocator);
                else value.AddMember(key, val, allocator);
                break;
            }
        }
    }

    void SystemNamespace::_writeJSONReadValuesFromTable(HSQUIRRELVM vm, rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator){
        sq_pushnull(vm);
        while(SQ_SUCCEEDED(sq_next(vm, -2))){
            const SQChar* foundLabel;
            SQRESULT result = sq_getstring(vm, -2, &foundLabel);
            assert(SQ_SUCCEEDED(result));
            rapidjson::Value key((char*)foundLabel, allocator);

            _writeJSONProcessValue(vm, key, value, allocator, false);

            sq_pop(vm, 2);
        }
        sq_pop(vm, 1);
    }

    SQInteger SystemNamespace::writeTableAsJsonFile(HSQUIRRELVM vm){
        const SQChar* outVal;
        sq_getstring(vm, 2, &outVal);

        bool prettyPrint = true;
        if(sq_gettop(vm) >= 4){
            SQBool b;
            sq_getbool(vm, 4, &b);
            //Reset so the bool, and potentially any other values, so the stack is clean to parse the table.
            sq_settop(vm, 3);
            prettyPrint = b;
        }

        rapidjson::Document d;
        rapidjson::GenericValue<rapidjson::UTF8<>>& value = d.SetObject();
        rapidjson::Document::AllocatorType& allocator = d.GetAllocator();

        _writeJSONReadValuesFromTable(vm, value, allocator);

        FILE* fp = fopen(outVal, "w"); // non-Windows use "w"
        if(!fp) return sq_throwerror(vm, "Error opening file.");
        char writeBuffer[65536];
        rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

        if(prettyPrint){
            rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
            d.Accept(writer);
        }else{
            rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
            d.Accept(writer);
        }
        fclose(fp);

        return 0;
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
        /**SQFunction
        @name writeJsonAsFile
        @desc Write a table object as a json file.
        */
        ScriptUtils::addFunction(vm, writeTableAsJsonFile, "writeJsonAsFile", -3, ".stb");

    }
}
