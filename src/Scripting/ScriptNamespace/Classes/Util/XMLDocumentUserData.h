#pragma once

#include "tinyxml2.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class XMLDocumentUserData{
    public:
        XMLDocumentUserData() = delete;
        ~XMLDocumentUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void XMLDocumentToUserData(HSQUIRRELVM vm, tinyxml2::XMLDocument* doc);
        static UserDataGetResult readXMLDocumentFromUserData(HSQUIRRELVM vm, SQInteger stackInx, tinyxml2::XMLDocument** doc);

    private:
        static SQInteger getRootElement(HSQUIRRELVM vm);
        static SQInteger createXMLDocument(HSQUIRRELVM vm);
        static SQInteger loadFile(HSQUIRRELVM vm);
        static SQInteger writeFile(HSQUIRRELVM vm);
        static SQInteger newElement(HSQUIRRELVM vm);

        static SQObject XMLDocumentDelegateTableObject;
    };
}
