#pragma once

#include <map>
#include <string>
#include "OgreIdString.h"
#include "Dialog/Compiler/DialogScriptData.h"

namespace AV{

    struct ConstantVariableAttribute{
        VariableAttribute a;
        //Store the string as well as everything with the variable attribute.
        std::string s;
    };

    typedef std::map<Ogre::IdString,ConstantVariableAttribute> DialogConstantMap;

    /**
    Simple settings for the current dialog execution.
    */
    class DialogSettings{
        friend class SystemSetup;

    private:
        //Note. Modifying this after startup may not be thread safe if dialog script parsing is threaded.
        static DialogConstantMap mDialogConstantsMap;

    public:
        static const DialogConstantMap& getDialogConstantsMap() { return mDialogConstantsMap; }
    };
}
