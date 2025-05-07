#pragma once

#ifdef TARGET_ANDROID
    #include <android/asset_manager.h>
    #include <android/asset_manager_jni.h>
#endif

#include <filesystem>

namespace AV{

    class AndroidSetupHelper;

    class FilePath{
        friend AndroidSetupHelper;
    public:
        FilePath();
        ~FilePath();
        FilePath(const std::filesystem::path& p);

        bool exists() const;
        bool is_file() const;

        FilePath operator/(const FilePath& p) const;
        FilePath& operator/=(const FilePath& p);
        FilePath& operator=(const FilePath& p);

        std::string string() const;
        std::string str() const;
        FilePath parent_path() const;
        bool is_absolute() const;
        bool is_directory() const;
        FilePath make_absolute() const;

        const std::filesystem::path getStdPath() const;

    //TODO put this back to private.
    public:
        std::filesystem::path mPath;

        #ifdef TARGET_ANDROID
            static AAssetManager* mAssetManager;
        #endif
    };

}
