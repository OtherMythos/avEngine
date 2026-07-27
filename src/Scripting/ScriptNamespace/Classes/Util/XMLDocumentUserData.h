#pragma once

#include "tinyxml2.h"

#include <memory>

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    /**
    Documents are shared rather than owned outright by the userdata that created them.
    Element userdata holds a reference to the document it came from, so a document stays alive
    for as long as any element pointing into it does.
    */
    typedef std::shared_ptr<tinyxml2::XMLDocument> XMLDocumentPtr;

    class XMLDocumentUserData{
    public:
        XMLDocumentUserData() = delete;
        ~XMLDocumentUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void XMLDocumentToUserData(HSQUIRRELVM vm, XMLDocumentPtr doc);
        static UserDataGetResult readXMLDocumentFromUserData(HSQUIRRELVM vm, SQInteger stackInx, XMLDocumentPtr* doc);

    private:
        static SQInteger getRootElement(HSQUIRRELVM vm);
        static SQInteger createXMLDocument(HSQUIRRELVM vm);
        static SQInteger loadFile(HSQUIRRELVM vm);
        static SQInteger writeFile(HSQUIRRELVM vm);
        static SQInteger newElement(HSQUIRRELVM vm);

        static SQInteger documentReleaseHook(SQUserPointer p, SQInteger size);

        static SQObject XMLDocumentDelegateTableObject;
    };
}
