#include "GuiNamespace.h"

#include "System/BaseSingleton.h"

#include "ColibriGui/ColibriManager.h"
#include "ColibriGui/ColibriWindow.h"
#include "Gui/GuiManager.h"
#include "Scripting/ScriptObjectTypeTags.h"

#include <vector>

namespace AV{

    static std::vector<Colibri::Widget*> _storedPointers;
    static std::vector<GuiNamespace::WidgetVersion> _storedVersions;

    SQInteger GuiNamespace::createWindow(HSQUIRRELVM vm){

        Colibri::Window* win = BaseSingleton::getGuiManager()->getColibriManager()->createWindow(0);

        WidgetId id = _storeWidget(win);
        _widgetIdToUserData(vm, id);

        return 1;
    }



    void GuiNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, createWindow, "createWindow");
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
        sq_settypetag(vm, -1, WidgetWindow);
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

    Colibri::Widget* GuiNamespace::_getWidget(uint64_t id){
        return 0;
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
