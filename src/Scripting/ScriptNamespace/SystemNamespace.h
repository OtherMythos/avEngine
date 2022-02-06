#pragma once

#include "ScriptUtils.h"
#include "rapidjson/document.h"

namespace AV{
    class ScriptVM;
    class SystemNamespace{
        friend ScriptVM;
    public:
        SystemNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:

        static SQInteger makeDirectory(HSQUIRRELVM vm);
        static SQInteger readJSONAsTable(HSQUIRRELVM vm);
        static SQInteger writeTableAsJsonFile(HSQUIRRELVM vm);

        static void _readJsonObject(HSQUIRRELVM vm, const rapidjson::GenericMember<rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<>>& value);
        static void _readJsonValue(HSQUIRRELVM vm, const rapidjson::Value& value);

        static void _writeJSONReadValuesFromTable(HSQUIRRELVM vm, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator);
        static void _writeJSONProcessValue(HSQUIRRELVM vm, rapidjson::Value& key, rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator, bool isArray);
    };
}
