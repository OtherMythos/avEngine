#pragma once

#include "System/SystemSetup/SystemSettings.h"
#include <sys/stat.h>

#ifdef _WIN32
    #include "filesystem/path.h"
#endif

namespace AV{

    static const std::string resHeader = "res://";

    /**
    Take a res path and convert it into an absolute path.
    */
    static void formatResToPath(const std::string& path, std::string& outPath){
        outPath = path;
        if(path.rfind(resHeader, 0) == 0) { //Has the res header at the beginning.
            outPath.replace(0, 6, SystemSettings::getDataPath());
        }
    }

    static bool fileExists(const std::string& path){
        #if defined(_WIN32)
            //return GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES;
            //Windows is a bloody nightmare and I couldn't figure out how to convert a const char to whatever the windows api function uses.
            //So this is what it gets for now.
            return filesystem::path(path).exists();
        #else
            struct stat sb;
            return stat(path.c_str(), &sb) == 0;
        #endif
    }
}
