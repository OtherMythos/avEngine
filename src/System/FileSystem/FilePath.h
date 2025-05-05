#pragma once

#include <filesystem>

namespace AV{

    class FilePath{
    public:
        FilePath();
        ~FilePath();
        FilePath(const std::filesystem::path& p);

        bool exists() const;
        bool is_file() const;

        FilePath operator/(const FilePath& p);
        FilePath& operator/=(const FilePath& p);
        FilePath& operator=(const FilePath& p);

        std::string string() const;
        std::string str() const;

    private:
        std::filesystem::path mPath;
    };

}
