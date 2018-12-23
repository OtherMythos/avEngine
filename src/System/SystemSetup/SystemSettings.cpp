#include "SystemSettings.h"

namespace AV {
    std::string SystemSettings::_dataPath = ".";
    std::string SystemSettings::_masterPath = ".";
    
    std::string SystemSettings::_windowTitle = "AV Engine";
    Ogre::ColourValue SystemSettings::_compositorColour = Ogre::ColourValue::Black;
    
    bool SystemSettings::_ogreResourcesFileViable = false;
}
