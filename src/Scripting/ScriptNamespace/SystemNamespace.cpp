#include "SystemNamespace.h"

#include "System/Util/PathUtils.h"
#include "filesystem/path.h"
#include <fstream>

#include "rapidjson/filereadstream.h"
#include "rapidjson/error/en.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"

#include <filesystem>

#include <time.h>

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

    SQInteger SystemNamespace::pathExists(HSQUIRRELVM vm){
        const SQChar *path;
        sq_getstring(vm, 2, &path);

        std::string outString;
        formatResToPath(path, outString);

        bool exists = fileExists(outString);
        sq_pushbool(vm, exists);

        return 1;
    }

    SQInteger SystemNamespace::createBlankFile(HSQUIRRELVM vm){
        const SQChar *path;
        sq_getstring(vm, 2, &path);

        std::string outString;
        formatResToPath(path, outString);

        std::ofstream output(outString);
        if(!output.good()){
            std::string s("Error creating file at path ");
            s += outString;
            return sq_throwerror(vm, s.c_str());
        }

        return 1;
    }

    SQInteger SystemNamespace::ensureUserDirectory(HSQUIRRELVM vm){
        const filesystem::path dirPath(SystemSettings::getUserDirectoryPath());
        if(!dirPath.exists()){
            filesystem::create_directory(dirPath);
        }

        return 0;
    }

    SQInteger SystemNamespace::resolveResPath(HSQUIRRELVM vm){
        const SQChar *path;
        sq_getstring(vm, 2, &path);

        std::string outString;
        formatResToPath(path, outString);

        sq_pushstring(vm, outString.c_str(), outString.size());

        return 1;
    }

    SQInteger SystemNamespace::getFilenamePath(HSQUIRRELVM vm){
        const SQChar *path;
        sq_getstring(vm, 2, &path);

        const std::filesystem::path dirPath(path);
        const std::string outPath = dirPath.filename().string();

        sq_pushstring(vm, outPath.c_str(), outPath.size());

        return 1;
    }

    SQInteger SystemNamespace::getParentPath(HSQUIRRELVM vm){
        const SQChar *path;
        sq_getstring(vm, 2, &path);

        const std::filesystem::path dirPath(path);
        const std::string outPath = dirPath.parent_path().string();

        sq_pushstring(vm, outPath.c_str(), outPath.size());

        return 1;
    }

    SQInteger SystemNamespace::getFilesInDirectory(HSQUIRRELVM vm){
        const SQChar *path;
        sq_getstring(vm, 2, &path);

        std::string outString;
        formatResToPath(path, outString);

        std::vector<std::string> entries;
        //using recursive_directory_iterator = std::filesystem::directory_iterator;
        for(const std::filesystem::directory_entry& dirEntry : std::filesystem::directory_iterator(outString)){
            const std::string name = dirEntry.path().filename();
            entries.push_back(name);
        }

        sq_newarray(vm, 0);
        for(const std::string& i : entries){
            sq_pushstring(vm, i.c_str(), i.size());
            sq_arrayappend(vm, -2);
        }

        return 1;
    }

    SQInteger SystemNamespace::removeFile(HSQUIRRELVM vm){
        const SQChar *path;
        sq_getstring(vm, 2, &path);

        std::string outString;
        formatResToPath(path, outString);

        WRAP_STD_ERROR(
            std::filesystem::remove(outString);
        )

        return 0;
    }

    SQInteger SystemNamespace::removeAll(HSQUIRRELVM vm){
        const SQChar *path;
        sq_getstring(vm, 2, &path);

        std::string outString;
        formatResToPath(path, outString);

        WRAP_STD_ERROR(
            std::filesystem::remove_all(outString);
        )

        return 0;
    }


    SQInteger SystemNamespace::renameFile(HSQUIRRELVM vm){
        const SQChar *path;
        sq_getstring(vm, 2, &path);
        std::string firstPath;
        formatResToPath(path, firstPath);

        sq_getstring(vm, 3, &path);
        std::string secondPath;
        formatResToPath(path, secondPath);

        WRAP_STD_ERROR(
            std::filesystem::rename(firstPath, secondPath);
        )

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

        if(!fileExists(outString)){
            return sq_throwerror(vm, "File does not exist.");
        }

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

        std::string outString;
        formatResToPath(outVal, outString);

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

        FILE* fp = fopen(outString.c_str(), "w"); // non-Windows use "w"
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

    void _setIntegerSlot(HSQUIRRELVM v, const SQChar *name, SQInteger val){
        sq_pushstring(v,name,-1);
        sq_pushinteger(v,val);
        sq_rawset(v,-3);
    }

    SQInteger SystemNamespace::getDate(HSQUIRRELVM vm){
        ScriptUtils::_debugStack(vm);
        time_t t;
        SQInteger it;
        SQInteger format = (SQInteger)TimeFormat::LOCAL;
        if(sq_gettop(vm) > 1){
            sq_getinteger(vm, 2, &it);
            t = it;
            if(sq_gettop(vm) > 2){
                sq_getinteger(vm, 3, (SQInteger*)&format);
            }
        }
        else{
            time(&t);
        }
        tm *date;
        if(format == (SQInteger)TimeFormat::UTC)
            date = gmtime(&t);
        else
            date = localtime(&t);
        if(!date)
            return sq_throwerror(vm,_SC("crt api failure"));
        sq_newtable(vm);
        _setIntegerSlot(vm, _SC("sec"), date->tm_sec);
        _setIntegerSlot(vm, _SC("min"), date->tm_min);
        _setIntegerSlot(vm, _SC("hour"), date->tm_hour);
        _setIntegerSlot(vm, _SC("day"), date->tm_mday);
        _setIntegerSlot(vm, _SC("month"), date->tm_mon);
        _setIntegerSlot(vm, _SC("year"), date->tm_year+1900);
        _setIntegerSlot(vm, _SC("wday"), date->tm_wday);
        _setIntegerSlot(vm, _SC("yday"), date->tm_yday);
        return 1;
    }

    SQInteger SystemNamespace::getTime(HSQUIRRELVM vm){
        SQInteger t = (SQInteger)time(NULL);
        sq_pushinteger(vm, t);
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
        @name exists
        @desc Check if the provided resPath exists on the file system.
        */
        ScriptUtils::addFunction(vm, pathExists, "exists", 2, ".s");
        /**SQFunction
        @name remove
        @desc Remove a file at the provided resPath.
        */
        ScriptUtils::addFunction(vm, removeFile, "remove", 2, ".s");
        /**SQFunction
        @name removeAll
        @desc Recursively remove files and directories.
        */
        ScriptUtils::addFunction(vm, removeAll, "removeAll", 2, ".s");
        /**SQFunction
        @name rename
        @desc Rename or move a file.
        */
        ScriptUtils::addFunction(vm, renameFile, "rename", 3, ".ss");

        /**SQFunction
        @name createBlankFile
        @desc resPath to blank file to be created
        */
        ScriptUtils::addFunction(vm, createBlankFile, "createBlankFile", 2, ".s");
        /**SQFunction
        @name ensureUserDirectory
        @desc Ensure the user directory exists for this project and user:// paths can be used.
        */
        ScriptUtils::addFunction(vm, ensureUserDirectory, "ensureUserDirectory");
        /**SQFunction
        @name resolveResPath
        @desc Resolve a path which begins with either res:// or user:// to an absolute path
        */
        ScriptUtils::addFunction(vm, resolveResPath, "resolveResPath");

        /**SQFunction
        @name getParentPath
        @desc Determine the parent path of a given path.
        */
        ScriptUtils::addFunction(vm, getParentPath, "getParentPath");
        /**SQFunction
        @name getFilenamePath
        @desc Return the filename component of a path.
        */
        ScriptUtils::addFunction(vm, getFilenamePath, "getFilenamePath");
        /**SQFunction
        @name getFilesInDirectory
        @desc Returns an array of all files in the given directory path.
        */
        ScriptUtils::addFunction(vm, getFilesInDirectory, "getFilesInDirectory", 2, ".s");
        /**SQFunction
        @name readJSONAsTable
        @desc Read a json file, returning the json data as a table object.
        */
        ScriptUtils::addFunction(vm, readJSONAsTable, "readJSONAsTable", 2, ".s");
        /**SQFunction
        @name writeJsonAsFile
        @desc Write a table object as a json file.
        */
        ScriptUtils::addFunction(vm, writeTableAsJsonFile, "writeJsonAsFile", -3, ".stb");
        /**SQFunction
        @name date
        @desc Return the date as a table.
        //TODO could do with a proper table.
        */
        ScriptUtils::addFunction(vm, getDate, "date", -1, ".nn");
        /**SQFunction
        @name time
        @desc Get the current time in seconds since the 1970s epoc.
        */
        ScriptUtils::addFunction(vm, getTime, "time");
    }

    void SystemNamespace::setupConstants(HSQUIRRELVM vm){
        ScriptUtils::declareConstant(vm, "_TIME_FORMAT_LOCAL", (SQInteger)TimeFormat::LOCAL);
        ScriptUtils::declareConstant(vm, "_TIME_FORMAT_UTC", (SQInteger)TimeFormat::UTC);
    }
}
