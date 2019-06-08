#pragma once

#include <vector>

namespace AV{
    /**
    A class to allow easier metadata construction and storage for squirrel script objects.
    Say for instance a squirrel class instance wanted to store things like a vector3, or something more complicated, it would need to exist in memory somewhere.
    This class makes that easier to do, without having to allocate memory in the heap on the fly.
    */
    template <class T>
    class ScriptDataPacker{
    public:
        ScriptDataPacker(){

        }

        ~ScriptDataPacker(){

        }

        /**
        Get an entry from the list.
        */
        T& getEntry(void* entry){
            size_t index = reinterpret_cast<size_t>(entry);
            return mDataVec[index].val;
        }

        /**
        Store an entry in the list.

        @returns
        An identifier for that stored entry.

        @remarks
        The returned void* is NOT a valid pointer.
        It is intended to be used as a user pointer for a squirrel object, thus being formatted as a void pointer.
        Much of the api for this class accepts this returned void pointer as an identifier for this stored entry.
        */
        void* storeEntry(const T& obj){
            size_t outputIndex = 0;
            int targetPos = _determineListPosition();

            _entry e;
            e.val = obj;
            if(targetPos == -1) {
                outputIndex = mDataVec.size();
                mDataVec.push_back(e);
            }else{
                mDataVec[targetPos] = e;
                outputIndex = targetPos;
            }

            void* retVal = reinterpret_cast<void*>(outputIndex);

            return retVal;
        }

        void removeEntry(void* entry){
            size_t index = reinterpret_cast<size_t>(entry);

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
                    int foundIndex = mDataVec[currentSearchIndex].next;

                    if(foundIndex == -1){
                        //There is no preceding hole.
                        mDataVec[currentSearchIndex].next = index;
                        removalIndex = -1;

                        break;
                    }else if(foundIndex < index){
                        previousSearchIndex = currentSearchIndex;
                        currentSearchIndex = foundIndex;
                    }else if(foundIndex > index){
                        //The currect hole has been found.
                        mDataVec[currentSearchIndex].next = index;
                        removalIndex = foundIndex;

                        break;
                    }
                }
            }

            //Set the value in the list.
            mDataVec[index].next = removalIndex;
        }

    private:
        union _entry{
            T val;

            int next;
        };

        /**
        Returns a position in the list where the shape should be inserted into.
        This will re-configer the first hole and other tracking variables if necessary.
        */
        int _determineListPosition(){
            if(mFirstHole >= 0){
                int currentHole = mFirstHole;

                //TODO update the first hole to be whatever this points to.

                return currentHole;
            }

            return -1;
        }

        int mFirstHole = -1;
        std::vector<_entry> mDataVec;
    };
}
