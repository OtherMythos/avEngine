#pragma once

#include <string>

namespace filesystem{
    class path;
}

namespace AV{
    class SaveHandle{
    public:
        SaveHandle();
        ~SaveHandle();

        std::string saveName;

        std::string determineSaveDirectory() const;
        std::string determineEntitySerialisedFile() const;
        std::string determineMeshSerialisedFile() const;
        std::string determineSaveInfoFile() const;

    private:
        void _createInitialPath(filesystem::path* p) const;
    };
}
