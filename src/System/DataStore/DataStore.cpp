#include "DataStore.h"

namespace AV{
    DataStore* DataStore::mInstance = nullptr;

    void DataStore::setInstance(DataStore* store){
        mInstance = store;
    }

    DataStore* DataStore::getInstance(){
        return mInstance;
    }
}
