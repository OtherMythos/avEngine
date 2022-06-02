#pragma once

#include "AvSceneFileParserInterface.h"

namespace AV{
    //Scene parser interface except it uses spdlog instead of cout.
    //This is only expected to be used in the avEngine.
    class AvSceneInterfaceLogger : public SimpleSceneFileParserInterface{
    public:
        AvSceneInterfaceLogger(Ogre::SceneManager* manager, Ogre::SceneNode* parentNode) : SimpleSceneFileParserInterface(manager, parentNode) { }
        void logError(const char* message){
            AV_ERROR("{}", message);
        }
        void log(const char* message){
            AV_INFO("{}", message);
        }
    };
}