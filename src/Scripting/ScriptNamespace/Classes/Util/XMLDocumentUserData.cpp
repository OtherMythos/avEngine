#include "XMLDocumentUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "XMLElementUserData.h"

#include "System/Util/PathUtils.h"

#include <sstream>

namespace AV{

    SQObject XMLDocumentUserData::XMLDocumentDelegateTableObject;

    void XMLDocumentUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 1);

        ScriptUtils::addFunction(vm, getRootElement, "getRootElement");
        ScriptUtils::addFunction(vm, loadFile, "loadFile", 2, ".s");

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
        tinyxml2::XMLDocument* xmlDoc = new tinyxml2::XMLDocument();

        XMLDocumentToUserData(vm, xmlDoc);

        return 1;
    }

    SQInteger XMLDocumentUserData::getRootElement(HSQUIRRELVM vm){
        tinyxml2::XMLDocument* doc;
        SCRIPT_ASSERT_RESULT(readXMLDocumentFromUserData(vm, 1, &doc));

        tinyxml2::XMLElement* elem = doc->RootElement();

        XMLElementUserData::XMLElementToUserData(vm, elem);

        return 1;
    }

    SQInteger XMLDocumentUserData::loadFile(HSQUIRRELVM vm){
        const SQChar *filePath;
        sq_getstring(vm, 2, &filePath);
        std::string outString;
        formatResToPath(filePath, outString);

        tinyxml2::XMLDocument* doc;
        SCRIPT_ASSERT_RESULT(readXMLDocumentFromUserData(vm, 1, &doc));

        tinyxml2::XMLError result = doc->LoadFile(outString.c_str());
        if(result != tinyxml2::XML_SUCCESS){
            return sq_throwerror(vm, doc->ErrorStr());
        }

        return 0;
    }

    void XMLDocumentUserData::XMLDocumentToUserData(HSQUIRRELVM vm, tinyxml2::XMLDocument* doc){
        tinyxml2::XMLDocument** pointer = (tinyxml2::XMLDocument**)sq_newuserdata(vm, sizeof(tinyxml2::XMLDocument*));
        *pointer = doc;

        sq_pushobject(vm, XMLDocumentDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, XMLDocumentTypeTag);
    }

    UserDataGetResult XMLDocumentUserData::readXMLDocumentFromUserData(HSQUIRRELVM vm, SQInteger stackInx, tinyxml2::XMLDocument** outDoc){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != XMLDocumentTypeTag){
            *outDoc = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outDoc = *((tinyxml2::XMLDocument**)pointer);

        return USER_DATA_GET_SUCCESS;
    }

}
