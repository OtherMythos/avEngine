#pragma once

#include "AvSceneFileParser.h"
#include "AvSceneParsedData.h"

#include <iostream>
#include <stack>

#include "OgreItem.h"
#include "OgreSceneManager.h"

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

        inline int _processName(const char* name){
            if(!name) return -1;
            size_t idx = mFile->strings.size();
            mFile->strings.push_back(name);
            return static_cast<int>(idx);
        }

    public:
        AvSceneFileForDataParserInterface(ParsedSceneFile* file)
            : idCount(0),
              mFile(file){

            mPrevParents.push(-1);
        }

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
            _checkNewParent(parent);
            int nameIdx = _processName(vals.name);
            mFile->objects.push_back({SceneObjectType::Empty});
            mFile->data.push_back({0, vals.pos, vals.scale, vals.orientation, vals.animIdx, nameIdx});

            return ++idCount;
        }
        int createMesh(int parent, const char* mesh, const ElementBasicValues& vals){
            _checkNewParent(parent);
            int nameIdx = _processName(vals.name);
            mFile->objects.push_back({SceneObjectType::Mesh});
            size_t idx = mFile->strings.size();
            mFile->strings.push_back(mesh);
            mFile->data.push_back({idx, vals.pos, vals.scale, vals.orientation, vals.animIdx, nameIdx});

            return ++idCount;
        }
        int createUser(int userId, int parent, const char* userValue, const ElementBasicValues& vals){
            SceneObjectType t = SceneObjectType::User1;
            SceneObjectType objTypeVals[] = {SceneObjectType::User0, SceneObjectType::User1, SceneObjectType::User2};
            _checkNewParent(parent);
            int nameIdx = _processName(vals.name);
            mFile->objects.push_back({objTypeVals[userId]});
            size_t idx = mFile->strings.size();
            mFile->strings.push_back(userValue);
            mFile->data.push_back({idx, vals.pos, vals.scale, vals.orientation, vals.animIdx, nameIdx});

            return ++idCount;
        }
    };
}
