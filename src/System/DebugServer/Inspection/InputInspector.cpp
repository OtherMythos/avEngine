#ifdef DEBUG_SERVER

#include "InputInspector.h"

#include "System/DebugServer/Input/InputPlayback.h"

#include "System/BaseSingleton.h"
#include "Input/InputManager.h"

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

        doc.AddMember("frame", playback.getFrameNumber(), allocator);

        rapidjson::Value active(rapidjson::kArrayType);
        for(const InputPlayback::ActiveEntry& entry : playback.getActive()){
            rapidjson::Value obj(rapidjson::kObjectType);
            obj.AddMember("input", rapidjson::Value(entry.description.c_str(), allocator), allocator);
            obj.AddMember("framesRemaining", entry.framesRemaining, allocator);
            active.PushBack(obj, allocator);
        }
        doc.AddMember("active", active, allocator);
    }
}

#endif
