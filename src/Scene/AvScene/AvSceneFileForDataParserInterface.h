#pragma once

#include "AvSceneFileParser.h"
#include "AvSceneParsedData.h"

#include <iostream>
#include <stack>
#include "Logger/Log.h"

namespace AV{
    /**
    Interface for parsing a scene file to a reuseable piece of data.
    This would be useful if constructing lots of the same scene at once.
    */
    class AvSceneFileForDataParserInterface : public AVSceneFileParserInterface{
    private:
        ParsedSceneFile* mFile;
        std::stack<int> mPrevParents;

        //int mPrevParent;
        int idCount;

        inline void _checkNewParent(int parent){
            if(mPrevParents.top() != parent){
                mPrevParents.push(parent);
                mFile->objects.push_back({SceneObjectType::Child});
            }
        }

        //Set the first time something makes the parsed data unusable. The parser's own control
        //flow only aborts on the errors it detects itself, so a problem found while recording
        //has to be carried out to the caller here instead.
        bool mError = false;
        std::string mErrorMessage;

        inline int _processName(const char* name){
            if(!name) return -1;
            size_t idx = mFile->strings.size();
            mFile->strings.push_back(name);
            return static_cast<int>(idx);
        }

        /**
        Push a single object, recording the tree position alongside it.

        The parser hands out ids by creation order, and one data entry is pushed per created
        object, so the data index of an object is always its id minus one. A parent id of 0 or
        less means the parser considered this object to sit at the root.
        */
        inline int _pushObject(SceneObjectType type, int parent, size_t idx, const ElementBasicValues& vals){
            _checkNewParent(parent);
            int nameIdx = _processName(vals.name);
            int tagIdx = _processName(vals.tag);

            const int parentIdx = parent > 0 ? parent - 1 : -1;
            const uint32 objectIdx = static_cast<uint32>(mFile->data.size());

            mFile->objects.push_back({type});
            mFile->data.push_back({idx, vals.pos, vals.scale, vals.orientation, vals.animIdx, nameIdx, tagIdx, parentIdx, type});

            mFile->childIndices.push_back({});
            if(parentIdx < 0) mFile->rootIndices.push_back(objectIdx);
            else mFile->childIndices[parentIdx].push_back(objectIdx);

            if(vals.tag){
                //A tag is what a scene is searched by, so a second object claiming one would
                //make the lookup silently return whichever of them was registered first.
                auto insertResult = mFile->tags.insert({vals.tag, objectIdx});
                if(!insertResult.second && !mError){
                    mError = true;
                    mErrorMessage = std::string("Duplicate tag '") + vals.tag + "' in scene file. Tags must be unique.";
                    logError(mErrorMessage.c_str());
                }
            }

            return ++idCount;
        }

    public:
        AvSceneFileForDataParserInterface(ParsedSceneFile* file)
            : idCount(0),
              mFile(file){

            mPrevParents.push(-1);
        }

        /**
        Whether something was found while recording that makes the parsed data unusable.
        Worth checking even when loadFile returned true.
        */
        bool hasError() const { return mError; }
        const std::string& getError() const { return mErrorMessage; }

        void reachedEndForParent(int parent){
            if(parent == mPrevParents.top()){
                mFile->objects.push_back({SceneObjectType::Term});
                mPrevParents.pop();
            }
        }

        void logError(const char* message){
            AV_ERROR("{}", message);
        }
        void log(const char* message){
            AV_INFO("{}", message);
        }

        int createEmpty(int parent, const ElementBasicValues& vals){
            return _pushObject(SceneObjectType::Empty, parent, 0, vals);
        }
        int createMesh(int parent, const char* mesh, const ElementBasicValues& vals){
            size_t idx = mFile->strings.size();
            mFile->strings.push_back(mesh);
            return _pushObject(SceneObjectType::Mesh, parent, idx, vals);
        }
        int createUser(int userId, int parent, const char* userValue, const ElementBasicValues& vals){
            SceneObjectType objTypeVals[] = {SceneObjectType::User0, SceneObjectType::User1, SceneObjectType::User2, SceneObjectType::User3, SceneObjectType::User4, SceneObjectType::User5, SceneObjectType::User6};
            SceneObjectType type = (userId >= 0 && userId <= 6) ? objTypeVals[userId] : SceneObjectType::User0;

            size_t idx = mFile->strings.size();
            mFile->strings.push_back(userValue);
            return _pushObject(type, parent, idx, vals);
        }
    };
}
