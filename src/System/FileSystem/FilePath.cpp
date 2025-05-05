#include "FilePath.h"

namespace AV{

    FilePath::FilePath(){

    }

    FilePath::FilePath(const std::filesystem::path& p){
        mPath = p;
    }

    FilePath::~FilePath(){

    }

    bool FilePath::exists() const{
        return std::filesystem::exists(mPath);
    }

    FilePath FilePath::operator/(const FilePath& p){
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
        return std::filesystem::is_regular_file(mPath);
    }
}
