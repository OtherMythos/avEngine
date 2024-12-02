#pragma once

#include "OgreLog.h"

namespace AV
{

    class OgreAVLogListener : public Ogre::LogListener
    {
    public:
        OgreAVLogListener();

        virtual void messageLogged(const Ogre::String& message, Ogre::LogMessageLevel lml, bool maskDebug, const Ogre::String &logName, bool& skipThisMessage );
    };
}
