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

        int createEmpty(int parent, const Ogre::Vector3& pos, const Ogre::Vector3& scale, const Ogre::Quaternion& orientation){
            _checkNewParent(parent);
            mFile->objects.push_back({SceneObjectType::Empty, pos, scale, orientation});

            return ++idCount;
        }
        int createMesh(int parent, const char* name, const char* mesh, const Ogre::Vector3& pos, const Ogre::Vector3& scale, const Ogre::Quaternion& orientation){
            _checkNewParent(parent);
            mFile->objects.push_back({SceneObjectType::Mesh, pos, scale, orientation});

            return ++idCount;
        }
    };
}
