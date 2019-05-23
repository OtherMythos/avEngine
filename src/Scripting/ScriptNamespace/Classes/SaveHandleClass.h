#pragma once

#include <squirrel.h>

namespace AV{
    class SaveHandle;
    
    /**
    A class to expose the SaveHandle class to squirrel.
    */
    class SaveHandleClass{
    public:
        SaveHandleClass() { };

        static void setupClass(HSQUIRRELVM vm);
        
        static void saveHandleToInstance(HSQUIRRELVM vm, const SaveHandle& handle);
        static SaveHandle instanceToSaveHandle(HSQUIRRELVM vm);
        
    private:
        static SQMemberHandle saveName;
        
        static SQObject classObject;
    };
}
