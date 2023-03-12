#include "OgreAVLogListener.h"

#include "Log.h"

namespace AV
{
    OgreAVLogListener::OgreAVLogListener() {}

    void OgreAVLogListener::messageLogged( const Ogre::String &message, Ogre::LogMessageLevel lml, bool maskDebug, const Ogre::String &logName, bool &skipThisMessage )
    {
        if( lml < Ogre::LML_CRITICAL )
        {
            AV_OGRE_INFO(message);
        }
        else
        {
            AV_OGRE_ERROR(message);
        }
    }
}
