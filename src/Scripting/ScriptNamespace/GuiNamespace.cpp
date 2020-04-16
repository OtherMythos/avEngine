#include "GuiNamespace.h"

#include "System/BaseSingleton.h"

#include "ColibriGui/ColibriManager.h"
#include "Gui/GuiManager.h"
#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/ScriptNamespace/Classes/Gui/GuiWidgetDelegate.h"
#include "Scripting/ScriptNamespace/Classes/Gui/GuiSizerDelegate.h"

#include "ColibriGui/ColibriWindow.h"
#include "ColibriGui/ColibriButton.h"
#include "ColibriGui/ColibriLabel.h"
#include "ColibriGui/Layouts/ColibriLayoutLine.h"

#include <vector>
#include <map>

namespace AV{

    class GuiNamespaceWidgetListener : public Colibri::WidgetListener{
    public:
        void notifyWidgetDestroyed(Colibri::Widget *widget){
            GuiNamespace::_notifyWidgetDestruction(widget);
        }
    };

    class GuiNamespaceWidgetActionListener : public Colibri::WidgetActionListener{
    public:
        void notifyWidgetAction(Colibri::Widget *widget, Colibri::Action::Action action){
            GuiNamespace::_notifyWidgetActionPerformed(widget, action);
        }
    };

    //A list of all the windows ever created, unless they've been properly destroyed. This is used for correct destruction of objects during engine shutdown.
    static std::vector<Colibri::Window*> _createdWindows;
    static std::vector<Colibri::Widget*> _storedPointers;
    static std::vector<GuiNamespace::WidgetVersion> _storedVersions;

    static SQObject windowDelegateTable;
    static SQObject buttonDelegateTable;
    static SQObject labelDelegateTable;
    static SQObject sizerLayoutLineDelegateTable;

    static GuiNamespaceWidgetListener mNamespaceWidgetListener;
    static GuiNamespaceWidgetActionListener mNamespaceWidgetActionListener;

    static std::map<GuiNamespace::WidgetId, SQObject> _attachedListeners;
    static HSQUIRRELVM _vm; //A static reference to the vm for the action callback functions.

    static const uint32_t _listenerMask = Colibri::Action::Cancel | Colibri::Action::Highlighted | Colibri::Action::Hold | Colibri::Action::PrimaryActionPerform | Colibri::Action::SecondaryActionPerform | Colibri::Action::ValueChanged;

    int GuiNamespace::getNumWindows(){
        return _createdWindows.size();
    }

    int GuiNamespace::getNumWidgets(){
        int count = 0;

        for(Colibri::Widget* w : _storedPointers){
            if(!w) continue;

            if(!w->isWindow()) count++;
        }

        return count;
    }

    SQInteger GuiNamespace::createWindow(HSQUIRRELVM vm){

        Colibri::Window* win = BaseSingleton::getGuiManager()->getColibriManager()->createWindow(0);

        WidgetId id = _storeWidget(win);
        win->mUserId = id;
        win->addListener(&mNamespaceWidgetListener);
        _widgetIdToUserData(vm, id, WidgetWindowTypeTag);
        _createdWindows.push_back(win);

        sq_pushobject(vm, windowDelegateTable);

        sq_setdelegate(vm, -2);

        return 1;
    }

    SQInteger GuiNamespace::createLayoutLine(HSQUIRRELVM vm){
        //They're just created on the heap for now. There's no tracking of these pointers in this class.
        //When the object goes out of scope teh release hook destroys it.
        Colibri::LayoutLine* line = new Colibri::LayoutLine(BaseSingleton::getGuiManager()->getColibriManager());

        Colibri::LayoutBase** pointer = (Colibri::LayoutBase**)sq_newuserdata(vm, sizeof(Colibri::LayoutBase*));
        *pointer = line;

        sq_setreleasehook(vm, -1, layoutReleaseHook);
        sq_settypetag(vm, -1, LayoutLineTypeTag);

        sq_pushobject(vm, sizerLayoutLineDelegateTable);

        sq_setdelegate(vm, -2);

        return 1;
    }

    SQInteger GuiNamespace::destroyWidget(HSQUIRRELVM vm){
        void* expectedType;
        Colibri::Widget* outWidget = 0;
        UserDataGetResult result = getWidgetFromUserData(vm, -1, &outWidget, &expectedType);
        if(result != USER_DATA_GET_SUCCESS) return sq_throwerror(vm, "error reading passed value.");
        if(!isTypeTagWidget(expectedType)) return sq_throwerror(vm, "Incorrect object type passed");
        if(!outWidget) return sq_throwerror(vm, "Object handle is invalid.");

        //The widget listener will make the call to unstore the widget.
        BaseSingleton::getGuiManager()->getColibriManager()->destroyWidget(outWidget);

        return 0;
    }


    void GuiNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::setupDelegateTable(vm, &windowDelegateTable, GuiWidgetDelegate::setupWindow);
        ScriptUtils::setupDelegateTable(vm, &buttonDelegateTable, GuiWidgetDelegate::setupButton);
        ScriptUtils::setupDelegateTable(vm, &labelDelegateTable, GuiWidgetDelegate::setupLabel);
        ScriptUtils::setupDelegateTable(vm, &sizerLayoutLineDelegateTable, GuiSizerDelegate::setupLayoutLine);


        ScriptUtils::addFunction(vm, createWindow, "createWindow");
        ScriptUtils::addFunction(vm, createLayoutLine, "createLayoutLine");
        ScriptUtils::addFunction(vm, destroyWidget, "destroy", 2, ".u");

        _vm = vm;
    }

    void GuiNamespace::_notifyWidgetDestruction(Colibri::Widget* widget){
        WidgetId id = widget->mUserId;
        _unstoreWidget(id);


        unbindWidgetListener(widget);

        if(widget->isWindow()){
            auto it = std::find(_createdWindows.begin(), _createdWindows.end(), widget);
            if(it != _createdWindows.end()){
                _createdWindows.erase(it);
            }
        }
    }

    void GuiNamespace::unbindWidgetListener(Colibri::Widget* widget){
        WidgetId id = widget->mUserId;

        auto it = _attachedListeners.find(id);
        if(it != _attachedListeners.end()){
            SQObject obj = (*it).second;
            _attachedListeners.erase(it);
            widget->removeActionListener(&mNamespaceWidgetActionListener, _listenerMask);


            sq_release(_vm, &obj);
        }
    }

    void GuiNamespace::registerWidgetListener(Colibri::Widget* widget, SQObject targetFunction){
        WidgetId id = widget->mUserId;
        if(!_isWidgetIdValid(id)) return;

        //It's now confirmed for storage, so increase the reference so it's not destroyed until its released.
        sq_addref(_vm, &targetFunction);
        _attachedListeners[id] = targetFunction;

        widget->addActionListener(&mNamespaceWidgetActionListener, _listenerMask);
    }

    void GuiNamespace::_notifyWidgetActionPerformed(Colibri::Widget* widget, Colibri::Action::Action action){
        WidgetId id = widget->mUserId;
        auto it = _attachedListeners.find(id);
        if(it == _attachedListeners.end()) return;

        sq_pushobject(_vm, (*it).second);
        sq_pushroottable(_vm); //In future I might want to use something other than the root table.

        _widgetIdToUserData(_vm, id, WidgetButtonTypeTag); //TODO I should not be assuming what type it is.
        sq_pushobject(_vm, buttonDelegateTable); //I'd like to move this somewhere else as well.
        sq_setdelegate(_vm, -2);

        sq_pushinteger(_vm, (int)action);

        SQInteger paramCount = 3;

        if(SQ_FAILED(sq_call(_vm, paramCount, false, true))){
            //return false;
        }
    }

    void GuiNamespace::destroyStoredWidgets(){
        Colibri::ColibriManager* man = BaseSingleton::getGuiManager()->getColibriManager();
        for(Colibri::Window* w : _createdWindows){
            if(!w) continue;
            //This function calls delete on the pointer, as well as all its children.
            man->destroyWindow(w);
        }
        _createdWindows.clear();
        _storedPointers.clear();
        _storedVersions.clear();
    }

    UserDataGetResult GuiNamespace::getLayoutFromUserData(HSQUIRRELVM vm, SQInteger idx, Colibri::LayoutBase** outValue){
        SQUserPointer pointer, typeTag;
        if(!SQ_SUCCEEDED(sq_getuserdata(vm, idx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != LayoutLineTypeTag){
            *outValue = 0;
            return USER_DATA_GET_INCORRECT_TYPE;
        }
        assert(pointer);

        Colibri::LayoutBase** p = static_cast<Colibri::LayoutBase**>(pointer);
        *outValue = *p;

        return USER_DATA_GET_SUCCESS;
    }

    UserDataGetResult GuiNamespace::getWidgetFromUserData(HSQUIRRELVM vm, SQInteger idx, Colibri::Widget** outValue, void** expectedType){
        WidgetId outId;
        UserDataGetResult result = _widgetIdFromUserData(vm, idx, &outId, expectedType);
        if(result != USER_DATA_GET_SUCCESS) return result;

        *outValue = GuiNamespace::_getWidget(outId);

        return result;
    }

    bool GuiNamespace::isTypeTagBasicWidget(void* tag){
        return
            tag == WidgetButtonTypeTag ||
            tag == WidgetLabelTypeTag
            ;
    }

    bool GuiNamespace::isTypeTagWidget(void* tag){
        return
            tag == WidgetButtonTypeTag ||
            tag == WidgetLabelTypeTag ||
            tag == WidgetWindowTypeTag
            ;
    }

    void GuiNamespace::createWidget(HSQUIRRELVM vm, Colibri::Widget* parentWidget, WidgetType type){
        Colibri::Widget* w = 0;
        Colibri::ColibriManager* man = BaseSingleton::getGuiManager()->getColibriManager();
        SQObject* targetTable = 0;
        void* typeTag = 0;
        switch(type){
            case WidgetType::Button:
                w = man->createWidget<Colibri::Button>(parentWidget);
                targetTable = &buttonDelegateTable;
                typeTag = WidgetButtonTypeTag;
                break;
            case WidgetType::Label:
                w = man->createWidget<Colibri::Label>(parentWidget);
                targetTable = &labelDelegateTable;
                typeTag = WidgetLabelTypeTag;
                break;
            default:
                assert(false);
                break;
        }

        WidgetId id = _storeWidget(w);
        w->mUserId = id;
        //OPTIMISATION Each widget seems to contain a vector for listeners, however I only need the one.
        //It seems to me like modifications to the library might help fix this.
        w->addListener(&mNamespaceWidgetListener);
        _widgetIdToUserData(vm, id, typeTag);

        sq_pushobject(vm, *targetTable);
        sq_setdelegate(vm, -2);
    }

    SQInteger GuiNamespace::widgetReleaseHook(SQUserPointer p, SQInteger size){

        //Read the id from the user data and remove it from the lists.
        WidgetId* id = (WidgetId*)p;
        //TODO update if this function isn't necessary.
        //_unstoreWidget(*id); //Don't unstore it over this. The widget is removed when it is destroyed.

        return 0;
    }

    SQInteger GuiNamespace::layoutReleaseHook(SQUserPointer p, SQInteger size){

        Colibri::LayoutBase** pointer = static_cast<Colibri::LayoutBase**>(p);
        delete *pointer;

        return 0;
    }

    void GuiNamespace::_widgetIdToUserData(HSQUIRRELVM vm, WidgetId id, void* typeTag){
        WidgetId* pointer = (WidgetId*)sq_newuserdata(vm, sizeof(WidgetId));
        *pointer = id;

        sq_setreleasehook(vm, -1, widgetReleaseHook);
        sq_settypetag(vm, -1, typeTag);
    }

    UserDataGetResult GuiNamespace::_widgetIdFromUserData(HSQUIRRELVM vm, SQInteger idx, WidgetId* outId, void** outTypeTag){
        SQUserPointer pointer, typeTag;
        if(!SQ_SUCCEEDED(sq_getuserdata(vm, idx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        //TODO in future this should be either removed or updated.
        /*if(expectedType && typeTag != expectedType){ //Don't check the tag type if 0 is passed in.
            *outId = 0;
            return USER_DATA_GET_INCORRECT_TYPE;
        }*/
        *outTypeTag = typeTag;
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

    Colibri::Widget* GuiNamespace::_unstoreWidget(WidgetId id){
        uint32_t index;
        WidgetVersion version;
        _readWidgetId(id, &index, &version);
        assert(_storedVersions[index] == version);

        Colibri::Widget* w = _storedPointers[index];
        _storedPointers[index] = 0;
        _storedVersions[index]++; //Increase it here so anything that stores a version becomes invalid.

        return w;
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
