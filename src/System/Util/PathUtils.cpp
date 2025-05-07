#include "PathUtils.h"

#include <filesystem>
#include <Scripting/ScriptVM.h>

#include "System/FileSystem/FilePath.h"

namespace AV{
    void formatResToPath(const std::string& path, std::string& outPath){
        outPath = path;
        if(path.rfind(resHeader, 0) == 0) { //Has the res header at the beginning.
            outPath.replace(0, 6, SystemSettings::getDataPath());
            return;
        }
        if(path.rfind(userHeader, 0) == 0) {
            outPath.replace(0, 7, SystemSettings::getUserDirectoryPath());
            return;
        }
        if(path.rfind(scriptHeader, 0) == 0) {
            //Determine the path to the current file.
            SQStackInfos stackInfo;
            ScriptVM::populateStackInfoLowestFrame(&stackInfo);
            const std::string reducedPath = std::filesystem::path(stackInfo.source).remove_filename().string();
            outPath.replace(0, 7, reducedPath);
            return;
        }
    }

    bool fileExists(const std::string& path){
        return AV::FilePath(path).exists();
    }
}
