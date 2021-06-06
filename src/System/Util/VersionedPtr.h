#pragma once

#include "VersionedDataPool.h"
#include <map>

namespace AV{

    /**
    A data structure which allows versioned storage to pointers, intended for use with script exposing.
    Object types which require the user to store a pointer but need safety can use this class.
    Pointers can be accessed and used while also having access to a versioned id which can tell if they've been destroyed before.
    This allows for safe usage of the pointers to script exposed objects.
    */
    template <class T>
    class VersionedPtr{
    public:

        uint64 storeEntry(const T& t){
            uint64 newId = 0;
            auto it = _existing.find(t);
            if(it == _existing.end()){
                //Push to the map.
                //Count the number of objects which reference this pointer.
                newId = _pool.storeEntry({1, t});
                _existing[t] = newId;
            }else{
                newId = (*it).second;
                //Just increment the counter.
                _dataEntry& data = _pool.getEntry(newId);
                data.count++;
            }

            return newId;
        }

        bool removeEntry(uint64 id){
            if(!_pool.isIdValid(id)) return false;
            _dataEntry& de = _pool.getEntry(id);
            assert(de.count > 0);
            de.count--;
            if(de.count == 0){
                bool removed = _pool.removeEntry(id);
                assert(removed);

                auto it = _existing.find(de.value);
                assert(it != _existing.end());
                _existing.erase(it);
                return true;
            }
            //Count was greater than 0 so was not destroyed.
            return false;
        }

        bool removeEntry(T t){
            //TODO OPTIMISATION Storing the tracked data in the texture somehow would mean I don't have to do this search each time.
            auto it = _existing.find(t);
            //Nothing to do.
            if(it == _existing.end()) return false;

            //TODO OPTIMISATION with this approach there is a chance I perform the search twice.
            return removeEntry(it->second);
        }

        bool isIdValid(uint64 id){
            return _pool.isIdValid(id);
        }

        T& getEntry(uint64 entry){
            return _pool.getEntry(entry);
        }

        bool doesPtrExist(T t) const{
            return _existing.find(t) != _existing.end();
        }

    private:
        struct _dataEntry{
            uint32 count;
            T value;
        };
        VersionedDataPool<_dataEntry> _pool;
        std::map<T, uint64> _existing;
    };
}
