#pragma once

#include "sds_fstream.h"
#if defined(TARGET_ANDROID)
    #include "sds_fstreamApk.h"
#endif

namespace AV{

    /**
    A simple generic class to read lines from a file.
    This helps wrap filesystem support for systems like android.
    */
    class SimpleFileParser{
    public:

        SimpleFileParser(const std::string& filePath);
        ~SimpleFileParser();

        bool isOpen();
        bool getLine(std::string& line);

    private:

        #if defined(TARGET_ANDROID)
            sds::fstreamApk mFile;
        #else
            sds::fstream mFile;
        #endif

    };

}

