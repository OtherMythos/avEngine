#pragma once

#include "tinyxml2.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class XMLElementUserData{
    public:
        XMLElementUserData() = delete;
        ~XMLElementUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void XMLElementToUserData(HSQUIRRELVM vm, tinyxml2::XMLElement* elem);
        static UserDataGetResult readXMLElementFromUserData(HSQUIRRELVM vm, SQInteger stackInx, tinyxml2::XMLElement** elem);

    private:
        static SQInteger getText(HSQUIRRELVM vm);
        static SQInteger getName(HSQUIRRELVM vm);
        static SQInteger nextSiblingElement(HSQUIRRELVM vm);
        static SQInteger getFirstChildElement(HSQUIRRELVM vm);
        static SQInteger getAttribute(HSQUIRRELVM vm);
        static SQInteger setAttribute(HSQUIRRELVM vm);
        static SQInteger insertNewChildElement(HSQUIRRELVM vm);
        static SQInteger hasChildren(HSQUIRRELVM vm);

        static SQObject XMLElementDelegateTableObject;
    };
}
