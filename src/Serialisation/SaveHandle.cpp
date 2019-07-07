#include "SaveHandle.h"

#include "filesystem/path.h"

#include "System/SystemSetup/SystemSettings.h"

namespace AV{
    SaveHandle::SaveHandle(){

    }

    SaveHandle::~SaveHandle(){

    }

    void SaveHandle::_createInitialPath(filesystem::path* p) const{
        filesystem::path initial(SystemSettings::getSaveDirectory());

        *p = initial / filesystem::path(saveName);
    }

    std::string SaveHandle::determineSaveDirectory() const{
        filesystem::path initial;
        _createInitialPath(&initial);

        return initial.str();
    }

    std::string SaveHandle::determineSaveInfoFile() const{
        filesystem::path initial;
        _createInitialPath(&initial);

        filesystem::path saveInfoPath(initial / filesystem::path("saveInfo.avSave"));

        return saveInfoPath.str();
    }

    std::string SaveHandle::determineEntitySerialisedFile() const{
        filesystem::path initial;
        _createInitialPath(&initial);

        return (initial / filesystem::path("entityInfo")).str();
    }

    std::string SaveHandle::determineMeshSerialisedFile() const{
        filesystem::path initial;
        _createInitialPath(&initial);

        return (initial / filesystem::path("meshInfo")).str();
    }
}
