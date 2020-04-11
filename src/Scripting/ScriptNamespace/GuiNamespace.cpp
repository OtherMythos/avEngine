#include "GuiNamespace.h"

#include "System/BaseSingleton.h"

#include "ColibriGui/ColibriManager.h"
#include "ColibriGui/ColibriWindow.h"
#include "Gui/GuiManager.h"
#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/ScriptNamespace/Classes/Gui/GuiWidgetDelegate.h"

#include <vector>

namespace AV{

    static std::vector<Colibri::Widget*> _storedPointers;
    static std::vector<GuiNamespace::WidgetVersion> _storedVersions;

    static SQObject windowDelegateTable;

    SQInteger GuiNamespace::createWindow(HSQUIRRELVM vm){

        Colibri::Window* win = BaseSingleton::getGuiManager()->getColibriManager()->createWindow(0);

        WidgetId id = _storeWidget(win);
        _widgetIdToUserData(vm, id);

        //In future I would have a check of the type requested.
        sq_pushobject(vm, windowDelegateTable);

        sq_setdelegate(vm, -2);

        return 1;
    }



    void GuiNamespace::setupNamespace(HSQUIRRELVM vm){
        GuiWidgetDelegate::setupTable(vm);

        sq_resetobject(&windowDelegateTable);
        sq_getstackobj(vm, -1, &windowDelegateTable);
        sq_addref(vm, &windowDelegateTable);
        sq_pop(vm, 1);


        ScriptUtils::addFunction(vm, createWindow, "createWindow");
    }

    UserDataGetResult GuiNamespace::getWidgetFromUserData(HSQUIRRELVM vm, SQInteger idx, Colibri::Widget** outValue){
        WidgetId outId;
        UserDataGetResult result = _widgetIdFromUserData(vm, idx, &outId);
        if(result != USER_DATA_GET_SUCCESS) return result;

        *outValue = GuiNamespace::_getWidget(outId);

        return result;
    }

    SQInteger GuiNamespace::widgetReleaseHook(SQUserPointer p, SQInteger size){

        //Read the id from the user data and remove it from the lists.
        WidgetId* id = (WidgetId*)p;
        _unstoreWidget(*id);

        return 0;
    }

    void GuiNamespace::_widgetIdToUserData(HSQUIRRELVM vm, WidgetId id){
        WidgetId* pointer = (WidgetId*)sq_newuserdata(vm, sizeof(WidgetId));
        *pointer = id;

        sq_setreleasehook(vm, -1, widgetReleaseHook);
        sq_settypetag(vm, -1, WidgetWindowTypeTag);
    }

    UserDataGetResult GuiNamespace::_widgetIdFromUserData(HSQUIRRELVM vm, SQInteger idx, WidgetId* outId){
        SQUserPointer pointer, typeTag;
        if(!SQ_SUCCEEDED(sq_getuserdata(vm, idx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != WidgetWindowTypeTag){
            *outId = 0;
            return USER_DATA_GET_INCORRECT_TYPE;
        }
        assert(pointer);

        WidgetId* p = static_cast<WidgetId*>(pointer);
        *outId = *p;

        return USER_DATA_GET_SUCCESS;
    }

    GuiNamespace::WidgetId GuiNamespace::_storeWidget(Colibri::Widget* widget){
        assert(_storedVersions.size() == _storedPointers.size()); //Each pointer should have a version

        //OPTIMISATION rather than searching the entire list linearly I could have some sort of skip list.
        for(uint32_t i = 0; i < _storedPointers.size(); i++){
            if(_storedPointers[i] != 0x0) continue;

            //An empty entry was found.
            _storedPointers[i] = widget;
            //I don't increase the version here. That's done during the deletion.

            return _produceWidgetId(i, _storedVersions[i]);
        }

        //No space in the list was found, so just create one.
        uint32_t idx = _storedPointers.size();
        _storedPointers.push_back(widget);
        _storedVersions.push_back(0);

        return _produceWidgetId(idx, 0);
    }

    void GuiNamespace::_unstoreWidget(WidgetId id){
        uint32_t index;
        WidgetVersion version;
        _readWidgetId(id, &index, &version);
        assert(_storedVersions[index] == version);

        _storedPointers[index] = 0;
        _storedVersions[index]++; //Increase it here so anything that stores a version becomes invalid.
    }

    Colibri::Widget* GuiNamespace::_getWidget(WidgetId id){
        if(!_isWidgetIdValid(id)) return 0;

        //It might be a bit slow having to do this twice for each id.
        uint32_t index;
        WidgetVersion version;
        _readWidgetId(id, &index, &version);
        return _storedPointers[index];
    }

    GuiNamespace::WidgetId GuiNamespace::_produceWidgetId(uint32_t index, WidgetVersion version){
        return (WidgetId(index) | WidgetId(version) << 32UL);
    }

    void GuiNamespace::_readWidgetId(WidgetId id, uint32_t* outIndex, WidgetVersion* outVersion){
        *outIndex = uint32_t(0) | id;
        *outVersion = uint32_t(0) | id >> 32UL;
    }

    inline bool GuiNamespace::_isWidgetIdValid(WidgetId id){
        uint32_t index;
        WidgetVersion version;
        _readWidgetId(id, &index, &version);

        assert(index < _storedPointers.size());
        assert(index < _storedVersions.size());
        return _storedVersions[index] == version;
    }

}
