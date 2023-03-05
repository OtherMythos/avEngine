#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "Ogre.h"

namespace AV{
    class OgreBufferUserData{
    public:
        OgreBufferUserData() = delete;
        ~OgreBufferUserData() = delete;

        enum class OgreBufferType{
            vertexBuffer,
            indexBuffer
        };
        struct OgreBufferData{
            union{
                Ogre::VertexBufferPacked* vertexBuffer;
                Ogre::IndexBufferPacked* indexBuffer;
            };
            OgreBufferType bufType;

            bool operator ==(const OgreBufferData &d) const{
                return vertexBuffer == d.vertexBuffer && bufType == d.bufType;
            }
            OgreBufferData& operator =(const OgreBufferData &d){
                vertexBuffer = d.vertexBuffer;
                bufType = d.bufType;

                return *this;
            }
        };

        static void setupDelegateTable(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

        static void OgreBufferToUserData(HSQUIRRELVM vm, const OgreBufferData* program);

        static UserDataGetResult readOgreBufferFromUserData(HSQUIRRELVM vm, SQInteger stackInx, OgreBufferData* outProg);

    private:
        static SQObject OgreBufferDelegateTableObject;

        static SQInteger OgreBufferToString(HSQUIRRELVM vm);
        static SQInteger OgreBufferCompare(HSQUIRRELVM vm);

        static SQInteger OgreBufferObjectReleaseHook(SQUserPointer p, SQInteger size);
    };
}
