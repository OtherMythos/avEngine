#include "CollisionWorldObject.h"

namespace AV{

    CollisionWorldObject::CollisionWorldObject(int worldId){
        mWorldId = worldId;
    }

    CollisionWorldObject::~CollisionWorldObject(){

    }

    uint64 CollisionWorldObject::determineEnterLeaveBits(CollisionPackedResult first, CollisionPackedResult second, std::set<CollisionPackedResult>& prevPairs) const {
        auto firstIt = prevPairs.find(first);
        auto secondIt = prevPairs.find(second);
        uint64 out = 0;
        if(firstIt != prevPairs.end()){
            //The collision is active.
            prevPairs.erase(firstIt);
        }
        else if(secondIt != prevPairs.end()){
            prevPairs.erase(secondIt);
        }else{
            //The collision just began.
            out |= 0x1;
        }

        return out;
    }
}
