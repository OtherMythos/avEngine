#pragma once

#include <string>
#include <vector>
#include "SaveHandle.h"

namespace rapidjson
{
    class CrtAllocator;
    template <typename> class MemoryPoolAllocator;
    template <typename> struct UTF8;
    //template <typename, typename, typename> class GenericDocument;
    //typedef GenericDocument< UTF8<char>, MemoryPoolAllocator<CrtAllocator>, CrtAllocator > Document;

    template <typename BaseAllocator> class MemoryPoolAllocator;
    template <typename Encoding, typename>  class GenericValue;
    typedef GenericValue<UTF8<char>, MemoryPoolAllocator<CrtAllocator> > Value;
}

namespace AV{
    class SerialisationManager{
    public:
        SerialisationManager();
        ~SerialisationManager();

        void scanForSaves();
        void createNewSave(const SaveHandle& handle, bool override = true);

        const std::vector<SaveHandle>& getAvailableSaves() { return mSaves; }

    private:
        void _scanSaveDirectory(const std::string& dirName);
        void _processSaveInfo(const std::string& filePath);

        std::vector<SaveHandle> mSaves;
    };
}
