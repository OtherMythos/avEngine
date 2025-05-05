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
        FilePath parent_path() const;
        bool is_absolute() const;
        bool is_directory() const;
        FilePath make_absolute() const;

        const std::filesystem::path getStdPath() const;

    private:
        std::filesystem::path mPath;
    };

}
