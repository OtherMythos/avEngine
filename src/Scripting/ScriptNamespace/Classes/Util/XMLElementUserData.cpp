#include "XMLElementUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "System/Util/PathUtils.h"

#include <sstream>

namespace AV{

    SQObject XMLElementUserData::XMLElementDelegateTableObject;

    void XMLElementUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 1);

        ScriptUtils::addFunction(vm, getText, "getText");
        ScriptUtils::addFunction(vm, getName, "getName");
        ScriptUtils::addFunction(vm, nextSiblingElement, "nextSiblingElement");
        ScriptUtils::addFunction(vm, getFirstChildElement, "getFirstChildElement");
        ScriptUtils::addFunction(vm, insertNewChildElement, "insertNewChildElement", 2, ".s");
        ScriptUtils::addFunction(vm, hasChildren, "hasChildren");
        ScriptUtils::addFunction(vm, getAttribute, "getAttribute", 2, ".s");

        sq_resetobject(&XMLElementDelegateTableObject);
        sq_getstackobj(vm, -1, &XMLElementDelegateTableObject);
        sq_addref(vm, &XMLElementDelegateTableObject);
        sq_pop(vm, 1);
    }

    SQInteger XMLElementUserData::hasChildren(HSQUIRRELVM vm){
        tinyxml2::XMLElement* elem;
        SCRIPT_ASSERT_RESULT(readXMLElementFromUserData(vm, 1, &elem));

        bool noChildren = elem->NoChildren();
        sq_pushbool(vm, !noChildren);

        return 1;
    }

    SQInteger XMLElementUserData::insertNewChildElement(HSQUIRRELVM vm){
        tinyxml2::XMLElement* elem;
        SCRIPT_ASSERT_RESULT(readXMLElementFromUserData(vm, 1, &elem));

        const SQChar *elemName;
        sq_getstring(vm, 2, &elemName);

        tinyxml2::XMLElement* newElem = elem->InsertNewChildElement(elemName);

        XMLElementToUserData(vm, newElem);

        return 1;
    }

    SQInteger XMLElementUserData::getAttribute(HSQUIRRELVM vm){
        const SQChar *attributeName;
        sq_getstring(vm, 2, &attributeName);

        tinyxml2::XMLElement* elem;
        SCRIPT_ASSERT_RESULT(readXMLElementFromUserData(vm, 1, &elem));

        const char* out = elem->Attribute(attributeName);
        if(out == 0){
            sq_pushnull(vm);
            return 1;
        }

        sq_pushstring(vm, out, -1);
        return 1;
    }

    SQInteger XMLElementUserData::nextSiblingElement(HSQUIRRELVM vm){
        tinyxml2::XMLElement* elem;
        SCRIPT_ASSERT_RESULT(readXMLElementFromUserData(vm, 1, &elem));

        tinyxml2::XMLElement* next = elem->NextSiblingElement();
        if(next == 0){
            sq_pushnull(vm);
            return 1;
        }
        XMLElementToUserData(vm, next);

        return 1;
    }

    SQInteger XMLElementUserData::getFirstChildElement(HSQUIRRELVM vm){
        tinyxml2::XMLElement* elem;
        SCRIPT_ASSERT_RESULT(readXMLElementFromUserData(vm, 1, &elem));

        tinyxml2::XMLElement* next = elem->FirstChildElement();
        if(next == 0){
            sq_pushnull(vm);
            return 1;
        }
        XMLElementToUserData(vm, next);

        return 1;
    }

    SQInteger XMLElementUserData::getName(HSQUIRRELVM vm){
        tinyxml2::XMLElement* elem;
        SCRIPT_ASSERT_RESULT(readXMLElementFromUserData(vm, 1, &elem));

        const char* text = elem->Name();
        sq_pushstring(vm, text, -1);

        return 1;
    }

    SQInteger XMLElementUserData::getText(HSQUIRRELVM vm){
        tinyxml2::XMLElement* elem;
        SCRIPT_ASSERT_RESULT(readXMLElementFromUserData(vm, 1, &elem));

        const char* text = elem->GetText();
        sq_pushstring(vm, text, -1);

        return 1;
    }

    void XMLElementUserData::XMLElementToUserData(HSQUIRRELVM vm, tinyxml2::XMLElement* elem){
        tinyxml2::XMLElement** pointer = (tinyxml2::XMLElement**)sq_newuserdata(vm, sizeof(tinyxml2::XMLElement*));
        *pointer = elem;

        sq_pushobject(vm, XMLElementDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, XMLElementTypeTag);
    }

    UserDataGetResult XMLElementUserData::readXMLElementFromUserData(HSQUIRRELVM vm, SQInteger stackInx, tinyxml2::XMLElement** outElem){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != XMLElementTypeTag){
            *outElem = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outElem = *((tinyxml2::XMLElement**)pointer);

        return USER_DATA_GET_SUCCESS;
    }

}
