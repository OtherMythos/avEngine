#include "FilePath.h"

namespace AV{

    #ifdef TARGET_ANDROID
        AAssetManager* FilePath::mAssetManager = 0;
    #endif

    FilePath::FilePath(){

    }

    FilePath::FilePath(const std::filesystem::path& p){
        mPath = p;
    }

    FilePath::~FilePath(){

    }

    bool FilePath::exists() const{
        #ifdef TARGET_ANDROID
            bool isFile = is_file();
            if(isFile) return true;
            bool isDirectory = is_directory();
            if(isDirectory) return true;
            return false;
        #else
            return std::filesystem::exists(mPath);
        #endif
    }

    FilePath FilePath::operator/(const FilePath& p) const{
        return FilePath(mPath / p.mPath);
    }

    FilePath& FilePath::operator/=(const FilePath& p){
        mPath /= p.mPath;
        return *this;
    }

    FilePath& FilePath::operator=(const FilePath& p){
        mPath = p.mPath;
        return *this;
    }

    std::string FilePath::string() const{
        return mPath.string();
    }

    std::string FilePath::str() const{
        return string();
    }

    bool FilePath::is_file() const{
        #ifdef TARGET_ANDROID
            //If it can be opened then it's a file.
            AAsset* asset = AAssetManager_open(mAssetManager, mPath.string().c_str(), AASSET_MODE_UNKNOWN);
            if (asset) {
                AAsset_close(asset);
                return true;
            }
            return false;
        #else
            return std::filesystem::is_regular_file(mPath);
        #endif
    }

    bool FilePath::is_directory() const{
        #ifdef TARGET_ANDROID
            AAssetDir* dir = AAssetManager_openDir(mAssetManager, mPath.string().c_str());
            if (dir == nullptr) {
                return false;
            }
            const char* filename = AAssetDir_getNextFileName(dir);
            AAssetDir_close(dir);
            return filename != nullptr;
        #else
            return std::filesystem::is_directory(mPath);
        #endif
    }

    bool FilePath::is_absolute() const{
        return mPath.is_absolute();
    }

    FilePath FilePath::make_absolute() const{
        #ifdef TARGET_ANDROID
            return *this;
        #else
            return FilePath(std::filesystem::absolute(mPath));
        #endif
    }

    const std::filesystem::path FilePath::getStdPath() const{
        return mPath;
    }

    FilePath FilePath::parent_path() const{
        return FilePath(mPath.parent_path());
    }
}
