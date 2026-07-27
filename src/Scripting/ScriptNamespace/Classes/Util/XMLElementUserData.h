#pragma once

#include "tinyxml2.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "XMLDocumentUserData.h"

namespace AV{
    /**
    What an element userdata holds. Elements are owned by their document, so the document
    reference is what keeps the pointer valid. Without it a script could drop the last reference
    to the document and carry on reading elements out of freed memory.
    */
    struct XMLElementData{
        tinyxml2::XMLElement* elem;
        XMLDocumentPtr doc;
    };

    class XMLElementUserData{
    public:
        XMLElementUserData() = delete;
        ~XMLElementUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void XMLElementToUserData(HSQUIRRELVM vm, tinyxml2::XMLElement* elem, XMLDocumentPtr doc);
        static UserDataGetResult readXMLElementFromUserData(HSQUIRRELVM vm, SQInteger stackInx, XMLElementData** data);

    private:
        static SQInteger getText(HSQUIRRELVM vm);
        static SQInteger getName(HSQUIRRELVM vm);
        static SQInteger nextSiblingElement(HSQUIRRELVM vm);
        static SQInteger getFirstChildElement(HSQUIRRELVM vm);
        static SQInteger getAttribute(HSQUIRRELVM vm);
        static SQInteger setAttribute(HSQUIRRELVM vm);
        static SQInteger insertNewChildElement(HSQUIRRELVM vm);
        static SQInteger hasChildren(HSQUIRRELVM vm);

        static SQInteger elementReleaseHook(SQUserPointer p, SQInteger size);

        static SQObject XMLElementDelegateTableObject;
    };
}
