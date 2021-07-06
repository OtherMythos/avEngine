#pragma once

#include "System/EnginePrerequisites.h"
#include "DataPool.h"
#include <vector>
#include <cassert>

namespace AV{

    /**
    A datapool with its values versioned each time they are used.
    This means no two entries will ever have the same id, which can be useful for exposing to scripts.
    As well as this it is possible to check whether an id is valid at any time.
    */
    template <class T>
    class VersionedDataPool{
    public:
        static const uint64 INVALID_DATA_ID = 0xFFFFFFFFFFFFFFFF;
    public:
        uint64 storeEntry(const T& t){
            uint64 outVal = 0;
            uint32 id = mDataPool.storeEntry(t);

            outVal |= id;

            uint32 foundVersion = 0;
            if(id < mVersions.size()){
                foundVersion = mVersions[id];
            }else{
                assert(id == mVersions.size());
                mVersions.push_back(0);
            }
            uint64 castVersion = static_cast<uint64>(foundVersion);
            outVal |= (castVersion << 32);

            return outVal;
        }

        bool removeEntry(uint64 id){
            uint32 foundId, foundVersion;
            getValues(id, &foundId, &foundVersion);
            if(!_idValid(foundVersion, foundId)) return false;
            mDataPool.removeEntry(foundId);
            mVersions[foundId]++;

            return true;
        }

        inline void getValues(uint64 id, uint32* outId, uint32* outVersion) const{
            *outVersion = static_cast<uint32>((id >> 32) & 0xffffffff);
            *outId = static_cast<uint32>(id);

            assert(*outId <= mVersions.size());
        }

        bool isIdValid(uint64 id) const{
            if(id == INVALID_DATA_ID) return false;
            uint32 foundId, foundVersion;
            getValues(id, &foundId, &foundVersion);

            return _idValid(foundVersion, foundId);
        }

        T& getEntry(uint64 entry){
            return mDataPool.getEntry( static_cast<uint32>(entry) );
        }
        const T& getEntry(uint64 entry) const{
            return mDataPool.getEntry( static_cast<uint32>(entry) );
        }
        T* getEntryCheck(uint64 entry){
            if(!isIdValid(entry)) return 0;
            return &(mDataPool.getEntry( static_cast<uint32>(entry) ));
        }

        const std::vector<uint32>& getVersions() const { return mVersions; }
        const std::vector<std::pair<T, int>>& getInternalData() const { return mDataPool.getInternalData(); }

    private:
        std::vector<uint32> mVersions;
        DataPool<T> mDataPool;

        inline bool _idValid(uint32 version, uint32 id) const { return version == mVersions[id]; }
    };
}
