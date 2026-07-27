#include "XMLElementUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "System/Util/PathUtils.h"

#include <new>
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
        ScriptUtils::addFunction(vm, setAttribute, "setAttribute", 3, ".ss");

        sq_resetobject(&XMLElementDelegateTableObject);
        sq_getstackobj(vm, -1, &XMLElementDelegateTableObject);
        sq_addref(vm, &XMLElementDelegateTableObject);
        sq_pop(vm, 1);
    }

    SQInteger XMLElementUserData::hasChildren(HSQUIRRELVM vm){
        XMLElementData* data;
        SCRIPT_ASSERT_RESULT(readXMLElementFromUserData(vm, 1, &data));

        bool noChildren = data->elem->NoChildren();
        sq_pushbool(vm, !noChildren);

        return 1;
    }

    SQInteger XMLElementUserData::insertNewChildElement(HSQUIRRELVM vm){
        XMLElementData* data;
        SCRIPT_ASSERT_RESULT(readXMLElementFromUserData(vm, 1, &data));

        const SQChar *elemName;
        sq_getstring(vm, 2, &elemName);

        tinyxml2::XMLElement* newElem = data->elem->InsertNewChildElement(elemName);

        XMLElementToUserData(vm, newElem, data->doc);

        return 1;
    }


    SQInteger XMLElementUserData::setAttribute(HSQUIRRELVM vm){
        const SQChar *attributeName;
        sq_getstring(vm, 2, &attributeName);

        const SQChar *attributeValue;
        sq_getstring(vm, 3, &attributeValue);

        XMLElementData* data;
        SCRIPT_ASSERT_RESULT(readXMLElementFromUserData(vm, 1, &data));

        data->elem->SetAttribute(attributeName, attributeValue);

        return 0;
    }

    SQInteger XMLElementUserData::getAttribute(HSQUIRRELVM vm){
        const SQChar *attributeName;
        sq_getstring(vm, 2, &attributeName);

        XMLElementData* data;
        SCRIPT_ASSERT_RESULT(readXMLElementFromUserData(vm, 1, &data));

        const char* out = data->elem->Attribute(attributeName);
        if(out == 0){
            sq_pushnull(vm);
            return 1;
        }

        sq_pushstring(vm, out, -1);
        return 1;
    }

    SQInteger XMLElementUserData::nextSiblingElement(HSQUIRRELVM vm){
        XMLElementData* data;
        SCRIPT_ASSERT_RESULT(readXMLElementFromUserData(vm, 1, &data));

        tinyxml2::XMLElement* next = data->elem->NextSiblingElement();
        if(next == 0){
            sq_pushnull(vm);
            return 1;
        }
        XMLElementToUserData(vm, next, data->doc);

        return 1;
    }

    SQInteger XMLElementUserData::getFirstChildElement(HSQUIRRELVM vm){
        XMLElementData* data;
        SCRIPT_ASSERT_RESULT(readXMLElementFromUserData(vm, 1, &data));

        tinyxml2::XMLElement* next = data->elem->FirstChildElement();
        if(next == 0){
            sq_pushnull(vm);
            return 1;
        }
        XMLElementToUserData(vm, next, data->doc);

        return 1;
    }

    SQInteger XMLElementUserData::getName(HSQUIRRELVM vm){
        XMLElementData* data;
        SCRIPT_ASSERT_RESULT(readXMLElementFromUserData(vm, 1, &data));

        const char* text = data->elem->Name();
        sq_pushstring(vm, text, -1);

        return 1;
    }

    SQInteger XMLElementUserData::getText(HSQUIRRELVM vm){
        XMLElementData* data;
        SCRIPT_ASSERT_RESULT(readXMLElementFromUserData(vm, 1, &data));

        const char* text = data->elem->GetText();
        sq_pushstring(vm, text, -1);

        return 1;
    }

    void XMLElementUserData::XMLElementToUserData(HSQUIRRELVM vm, tinyxml2::XMLElement* elem, XMLDocumentPtr doc){
        void* pointer = sq_newuserdata(vm, sizeof(XMLElementData));
        //The userdata memory is raw, so the entry is constructed into it by hand and destroyed
        //again by the release hook. Holding the document is what keeps elem valid.
        new (pointer) XMLElementData{elem, doc};
        sq_setreleasehook(vm, -1, elementReleaseHook);

        sq_pushobject(vm, XMLElementDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, XMLElementTypeTag);
    }

    SQInteger XMLElementUserData::elementReleaseHook(SQUserPointer p, SQInteger size){
        static_cast<XMLElementData*>(p)->~XMLElementData();

        return 0;
    }

    UserDataGetResult XMLElementUserData::readXMLElementFromUserData(HSQUIRRELVM vm, SQInteger stackInx, XMLElementData** outData){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != XMLElementTypeTag){
            *outData = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outData = static_cast<XMLElementData*>(pointer);

        return USER_DATA_GET_SUCCESS;
    }

}
