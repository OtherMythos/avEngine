#ifdef DEBUG_SERVER

#include "InputInspector.h"

#include "DebugJsonUtil.h"
#include "System/DebugServer/Input/InputPlayback.h"

#include "System/BaseSingleton.h"
#include "Input/InputManager.h"
#include "Input/InputRouter.h"

namespace AV{
    static void writeActionRange(rapidjson::Value& array, const std::vector<InputManager::ActionSetDataEntry>& data,
                                 size_t start, size_t end, const char* type, rapidjson::Document::AllocatorType& allocator){
        for(size_t i = start; i < end && i < data.size(); i++){
            rapidjson::Value action(rapidjson::kObjectType);
            action.AddMember("name", rapidjson::Value(data[i].first.c_str(), allocator), allocator);
            action.AddMember("type", rapidjson::Value(type, allocator), allocator);
            array.PushBack(action, allocator);
        }
    }

    void InputInspector::writeActions(rapidjson::Document& doc, int& status){
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();

        std::shared_ptr<InputManager> input = BaseSingleton::getInputManager();
        if(!input){
            status = 503;
            doc.AddMember("error", "input manager not available", allocator);
            return;
        }

        const std::vector<InputManager::ActionSetEntry>& sets = input->getActionSets();
        const std::vector<InputManager::ActionSetDataEntry>& data = input->getActionSetData();

        rapidjson::Value setsArray(rapidjson::kArrayType);
        for(const InputManager::ActionSetEntry& set : sets){
            rapidjson::Value setObj(rapidjson::kObjectType);
            setObj.AddMember("name", rapidjson::Value(set.actionSetName.c_str(), allocator), allocator);

            rapidjson::Value actions(rapidjson::kArrayType);
            writeActionRange(actions, data, set.buttonStart, set.buttonEnd, "button", allocator);
            writeActionRange(actions, data, set.analogTriggerStart, set.analogTriggerEnd, "trigger", allocator);
            writeActionRange(actions, data, set.stickStart, set.stickEnd, "axis", allocator);
            setObj.AddMember("actions", actions, allocator);

            setsArray.PushBack(setObj, allocator);
        }
        doc.AddMember("actionSets", setsArray, allocator);
    }

    void InputInspector::writeState(rapidjson::Document& doc, const InputPlayback& playback){
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();

        doc.AddMember("frame", DebugJsonUtil::uint64Value(playback.getFrameNumber()), allocator);

        rapidjson::Value active(rapidjson::kArrayType);
        for(const InputPlayback::ActiveEntry& entry : playback.getActive()){
            rapidjson::Value obj(rapidjson::kObjectType);
            obj.AddMember("input", rapidjson::Value(entry.description.c_str(), allocator), allocator);
            obj.AddMember("framesRemaining", entry.framesRemaining, allocator);
            active.PushBack(obj, allocator);
        }
        doc.AddMember("active", active, allocator);
    }

    void InputInspector::writeLayers(rapidjson::Document& doc, int& status){
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();

        std::shared_ptr<InputRouter> router = BaseSingleton::getInputRouter();
        if(!router){
            status = 503;
            doc.AddMember("error", "input router not available", allocator);
            return;
        }

        std::vector<InputRouter::LayerInfo> layers;
        router->getLayerInfo(&layers);

        rapidjson::Value layersArray(rapidjson::kArrayType);
        for(const InputRouter::LayerInfo& l : layers){
            rapidjson::Value obj(rapidjson::kObjectType);
            obj.AddMember("name", rapidjson::Value(l.name, allocator), allocator);
            obj.AddMember("priority", l.priority, allocator);
            obj.AddMember("enabled", l.enabled, allocator);
            obj.AddMember("live", l.live, allocator);
            obj.AddMember("external", l.external, allocator);
            obj.AddMember("ownsPointer", l.ownsPointer, allocator);
            obj.AddMember("ownedButtons", l.ownedButtons, allocator);
            obj.AddMember("ownedKeys", l.ownedKeys, allocator);
            layersArray.PushBack(obj, allocator);
        }
        doc.AddMember("layers", layersArray, allocator);

        doc.AddMember("pluginInputEnabled", router->getExternalLayersEnabled(), allocator);
        doc.AddMember("guiConsumesInput", router->getGuiConsumesInput(), allocator);
    }
}

#endif
