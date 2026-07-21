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

        /**
        Outcome of validating an upload() call. Separated from the script binding
        so the arithmetic can be unit tested without a render system.
        */
        enum class BufferUploadCheck{
            Success,
            ImmutableBuffer,   //BT_IMMUTABLE can never be uploaded to.
            EmptyBlob,
            MisalignedBlob,    //Blob size is not a whole number of elements.
            NegativeStart,
            NegativeCount,
            CountExceedsBlob,  //Asked to upload more elements than the blob holds.
            OutOfBounds        //elementStart + count runs past the end of the buffer.
        };

        /**
        Determine whether a blob of blobSizeBytes can be uploaded into a buffer at
        elementStart. requestedCount caps how many elements are taken from the
        front of the blob; pass 0 to use the whole blob. On success
        outElementCount receives the number of elements that will be written.
        */
        static BufferUploadCheck checkBufferUpload(size_t numElements, Ogre::uint32 bytesPerElement,
                                                   size_t blobSizeBytes, SQInteger elementStart,
                                                   SQInteger requestedCount,
                                                   Ogre::BufferType bufferType, size_t* outElementCount);

        static void setupDelegateTable(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

        static void OgreBufferToUserData(HSQUIRRELVM vm, const OgreBufferData* program);

        static UserDataGetResult readOgreBufferFromUserData(HSQUIRRELVM vm, SQInteger stackInx, OgreBufferData* outProg);

    private:
        static SQObject OgreBufferDelegateTableObject;

        static SQInteger OgreBufferToString(HSQUIRRELVM vm);
        static SQInteger OgreBufferCompare(HSQUIRRELVM vm);

        static SQInteger upload(HSQUIRRELVM vm);
        static SQInteger getNumElements(HSQUIRRELVM vm);
        static SQInteger getBytesPerElement(HSQUIRRELVM vm);

        static SQInteger OgreBufferObjectReleaseHook(SQUserPointer p, SQInteger size);
    };
}
