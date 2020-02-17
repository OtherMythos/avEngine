#pragma once

#include "System/SystemSetup/SystemSettings.h"
#include <sys/stat.h>

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

    static bool fileExists(const char* path){
        #if defined(_WIN32)
            //return GetFileAttributesW(wstr().c_str()) != INVALID_FILE_ATTRIBUTES;
            assert(false); //Not done yet. Needs testing.
        #else
            struct stat sb;
            return stat(path, &sb) == 0;
        #endif
    }
}
