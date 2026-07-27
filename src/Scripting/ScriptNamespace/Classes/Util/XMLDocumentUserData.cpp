#include "XMLDocumentUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "XMLElementUserData.h"

#include "System/Util/PathUtils.h"

#include <new>
#include <sstream>

namespace AV{

    SQObject XMLDocumentUserData::XMLDocumentDelegateTableObject;

    void XMLDocumentUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 1);

        ScriptUtils::addFunction(vm, getRootElement, "getRootElement");
        ScriptUtils::addFunction(vm, loadFile, "loadFile", 2, ".s");
        ScriptUtils::addFunction(vm, writeFile, "writeFile", 2, ".s");
        ScriptUtils::addFunction(vm, newElement, "newElement", 2, ".s");

        sq_resetobject(&XMLDocumentDelegateTableObject);
        sq_getstackobj(vm, -1, &XMLDocumentDelegateTableObject);
        sq_addref(vm, &XMLDocumentDelegateTableObject);
        sq_pop(vm, 1);

        //Create the creation functions.
        sq_pushroottable(vm);

        {
            ScriptUtils::addFunction(vm, createXMLDocument, "XMLDocument");
        }

        sq_pop(vm, 1);
    }

    SQInteger XMLDocumentUserData::createXMLDocument(HSQUIRRELVM vm){
        XMLDocumentToUserData(vm, std::make_shared<tinyxml2::XMLDocument>());

        return 1;
    }

    SQInteger XMLDocumentUserData::newElement(HSQUIRRELVM vm){
        XMLDocumentPtr doc;
        SCRIPT_ASSERT_RESULT(readXMLDocumentFromUserData(vm, 1, &doc));

        const SQChar *filePath;
        sq_getstring(vm, 2, &filePath);
        tinyxml2::XMLElement* elem = doc->NewElement(filePath);
        doc->InsertFirstChild(elem);

        XMLElementUserData::XMLElementToUserData(vm, elem, doc);

        return 1;
    }

    SQInteger XMLDocumentUserData::getRootElement(HSQUIRRELVM vm){
        XMLDocumentPtr doc;
        SCRIPT_ASSERT_RESULT(readXMLDocumentFromUserData(vm, 1, &doc));

        tinyxml2::XMLElement* elem = doc->RootElement();

        XMLElementUserData::XMLElementToUserData(vm, elem, doc);

        return 1;
    }

    SQInteger XMLDocumentUserData::writeFile(HSQUIRRELVM vm){
        const SQChar *filePath;
        sq_getstring(vm, 2, &filePath);
        std::string outString;
        formatResToPath(filePath, outString);

        XMLDocumentPtr doc;
        SCRIPT_ASSERT_RESULT(readXMLDocumentFromUserData(vm, 1, &doc));

        tinyxml2::XMLError result = doc->SaveFile(outString.c_str());
        if(result != tinyxml2::XML_SUCCESS){
            return sq_throwerror(vm, doc->ErrorStr());
        }

        return 0;
    }

    SQInteger XMLDocumentUserData::loadFile(HSQUIRRELVM vm){
        const SQChar *filePath;
        sq_getstring(vm, 2, &filePath);
        std::string outString;
        formatResToPath(filePath, outString);

        XMLDocumentPtr doc;
        SCRIPT_ASSERT_RESULT(readXMLDocumentFromUserData(vm, 1, &doc));

        tinyxml2::XMLError result = doc->LoadFile(outString.c_str());
        if(result != tinyxml2::XML_SUCCESS){
            return sq_throwerror(vm, doc->ErrorStr());
        }

        return 0;
    }

    void XMLDocumentUserData::XMLDocumentToUserData(HSQUIRRELVM vm, XMLDocumentPtr doc){
        void* pointer = sq_newuserdata(vm, sizeof(XMLDocumentPtr));
        //The userdata memory is raw, so the shared pointer is constructed into it by hand and
        //destroyed again by the release hook.
        new (pointer) XMLDocumentPtr(doc);
        sq_setreleasehook(vm, -1, documentReleaseHook);

        sq_pushobject(vm, XMLDocumentDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, XMLDocumentTypeTag);
    }

    SQInteger XMLDocumentUserData::documentReleaseHook(SQUserPointer p, SQInteger size){
        static_cast<XMLDocumentPtr*>(p)->~XMLDocumentPtr();

        return 0;
    }

    UserDataGetResult XMLDocumentUserData::readXMLDocumentFromUserData(HSQUIRRELVM vm, SQInteger stackInx, XMLDocumentPtr* outDoc){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != XMLDocumentTypeTag){
            outDoc->reset();
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outDoc = *static_cast<XMLDocumentPtr*>(pointer);

        return USER_DATA_GET_SUCCESS;
    }

}
