#pragma once

#include <vector>
#include <climits>
#include <cassert>
#include "System/EnginePrerequisites.h"
#include <cstddef>

namespace AV{

    /**
    A simple data pool implementation.
    Objects are stored inside a vector.
    */
    template <class T>
    class DataPool{
    public:
        typedef std::pair<T, int> DataEntry;

        static const uint32 POOL_INVALID_DATA_ID = 0xffffffff;
    public:
        DataPool(){ }

        ~DataPool(){ }

        void clear(){
            mDataVec.clear();
        }

        /**
        Get an entry from the list.
        */
        T& getEntry(uint32 entry){
            assert(entry != POOL_INVALID_DATA_ID);

            assert(entry < mDataVec.size());
            return mDataVec[entry].first;
        }

        void setEntry(uint32 entry, const T& obj){
            assert(entry != POOL_INVALID_DATA_ID);

            assert(entry < mDataVec.size());
            mDataVec[entry].first = obj;
        }

        /**
        Store an entry in the list.

        @returns
        An identifier for that stored entry.
        */
        uint32 storeEntry(const T& obj){
            size_t outputIndex = 0;
            int targetPos = _determineListPosition();

            if(targetPos == -1) {
                outputIndex = mDataVec.size();
                mDataVec.push_back({obj, -1});
            }else{
                mDataVec[targetPos] = {obj, -1};
                outputIndex = targetPos;
            }

            uint32 retVal = static_cast<uint32>(outputIndex);

            return retVal;
        }

        void removeEntry(uint32 index){
            assert(index != POOL_INVALID_DATA_ID);
            assert(index < mDataVec.size());

            int removalIndex = -1;
            if(index < mFirstHole){
                //Make it point to the next entry.
                removalIndex = mFirstHole;
                mFirstHole = index;
            }else if(mFirstHole < 0){
                //There is no hole in the vector, so create one.
                mFirstHole = index;
                removalIndex = -1;
            }else if(index > mFirstHole){
                //We now need to do a search to re-organise the list and find the hole.

                int previousSearchIndex = -1;
                int currentSearchIndex = mFirstHole;
                while(true){
                    int foundIndex = mDataVec[currentSearchIndex].second;

                    if(foundIndex == -1){
                        //There is no preceding hole.
                        mDataVec[currentSearchIndex].second = index;
                        removalIndex = -1;

                        break;
                    }else if(foundIndex < index){
                        previousSearchIndex = currentSearchIndex;
                        currentSearchIndex = foundIndex;
                    }else if(foundIndex > index){
                        //The currect hole has been found.
                        mDataVec[currentSearchIndex].second = index;
                        removalIndex = foundIndex;

                        break;
                    }
                }
            }

            //Set the value in the list.
            mDataVec[index].second = removalIndex;
        }

    private:
        /**
        Returns a position in the list where the shape should be inserted into.
        This will re-configer the first hole and other tracking variables if necessary.
        */
        int _determineListPosition(){
            if(mFirstHole >= 0){
                int currentHole = mFirstHole;

                mFirstHole = mDataVec[currentHole].second;

                return currentHole;
            }

            return -1;
        }

        int mFirstHole = -1;
        std::vector<DataEntry> mDataVec;

    public:
        const std::vector<DataEntry>& getInternalData() const { return mDataVec; }
    };
}
