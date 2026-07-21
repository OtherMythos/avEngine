#include "OgreBufferUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "Ogre.h"
#include "Vao/OgreIndexBufferPacked.h"
#include "Vao/OgreVertexBufferPacked.h"

#include <sstream>
#include <sqstdblob.h>

namespace AV{

    SQObject OgreBufferUserData::OgreBufferDelegateTableObject;

    //Both arms of the union derive from BufferPacked, which owns everything the
    //upload path needs (size, stride, buffer type).
    static Ogre::BufferPacked* _asBufferPacked(const OgreBufferUserData::OgreBufferData& data){
        return data.bufType == OgreBufferUserData::OgreBufferType::vertexBuffer
            ? static_cast<Ogre::BufferPacked*>(data.vertexBuffer)
            : static_cast<Ogre::BufferPacked*>(data.indexBuffer);
    }

    void OgreBufferUserData::OgreBufferToUserData(HSQUIRRELVM vm, const OgreBufferData* ogreBuffer){
        OgreBufferData* pointer = (OgreBufferData*)sq_newuserdata(vm, sizeof(OgreBufferData));
        *pointer = *ogreBuffer;

        sq_pushobject(vm, OgreBufferDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, OgreBufferObjectTypeTag);
    }

    UserDataGetResult OgreBufferUserData::readOgreBufferFromUserData(HSQUIRRELVM vm, SQInteger stackInx, OgreBufferData* outProg){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != OgreBufferObjectTypeTag){
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outProg = *((const OgreBufferData*)pointer);

        return USER_DATA_GET_SUCCESS;
    }

    OgreBufferUserData::BufferUploadCheck OgreBufferUserData::checkBufferUpload(
            size_t numElements, Ogre::uint32 bytesPerElement, size_t blobSizeBytes,
            SQInteger elementStart, Ogre::BufferType bufferType, size_t* outElementCount){

        *outElementCount = 0;

        if(bufferType == Ogre::BT_IMMUTABLE) return BufferUploadCheck::ImmutableBuffer;
        if(elementStart < 0) return BufferUploadCheck::NegativeStart;
        if(blobSizeBytes == 0) return BufferUploadCheck::EmptyBlob;
        if(bytesPerElement == 0 || blobSizeBytes % bytesPerElement != 0) return BufferUploadCheck::MisalignedBlob;

        const size_t elementCount = blobSizeBytes / bytesPerElement;
        const size_t start = static_cast<size_t>(elementStart);
        if(start > numElements || elementCount > numElements - start) return BufferUploadCheck::OutOfBounds;

        *outElementCount = elementCount;
        return BufferUploadCheck::Success;
    }

    SQInteger OgreBufferUserData::upload(HSQUIRRELVM vm){
        OgreBufferData data;
        SCRIPT_CHECK_RESULT(readOgreBufferFromUserData(vm, 1, &data));
        Ogre::BufferPacked* buffer = _asBufferPacked(data);

        SQUserPointer blobData = 0;
        if(SQ_FAILED(sqstd_getblob(vm, 2, &blobData))){
            return sq_throwerror(vm, "upload expects a blob of buffer data.");
        }
        const SQInteger blobSize = sqstd_getblobsize(vm, 2);

        SQInteger elementStart = 0;
        if(sq_gettop(vm) >= 3) sq_getinteger(vm, 3, &elementStart);

        size_t elementCount = 0;
        BufferUploadCheck check = checkBufferUpload(buffer->getNumElements(), buffer->getBytesPerElement(),
                                                    static_cast<size_t>(blobSize), elementStart,
                                                    buffer->getBufferType(), &elementCount);
        switch(check){
            case BufferUploadCheck::ImmutableBuffer:
                return sq_throwerror(vm, "This buffer is immutable and cannot be uploaded to.");
            case BufferUploadCheck::NegativeStart:
                return sq_throwerror(vm, "Element start must be >= 0.");
            case BufferUploadCheck::EmptyBlob:
                return sq_throwerror(vm, "Uploading an empty blob is forbidden.");
            case BufferUploadCheck::MisalignedBlob:
                return sq_throwerror(vm, "Blob size must be a whole number of buffer elements.");
            case BufferUploadCheck::OutOfBounds:
                return sq_throwerror(vm, "Upload would write past the end of the buffer.");
            case BufferUploadCheck::Success:
                break;
        }

        WRAP_OGRE_ERROR(
            buffer->upload(blobData, static_cast<size_t>(elementStart), elementCount);
        )

        return 0;
    }

    SQInteger OgreBufferUserData::getNumElements(HSQUIRRELVM vm){
        OgreBufferData data;
        SCRIPT_CHECK_RESULT(readOgreBufferFromUserData(vm, 1, &data));

        sq_pushinteger(vm, static_cast<SQInteger>(_asBufferPacked(data)->getNumElements()));

        return 1;
    }

    SQInteger OgreBufferUserData::getBytesPerElement(HSQUIRRELVM vm){
        OgreBufferData data;
        SCRIPT_CHECK_RESULT(readOgreBufferFromUserData(vm, 1, &data));

        sq_pushinteger(vm, static_cast<SQInteger>(_asBufferPacked(data)->getBytesPerElement()));

        return 1;
    }

    SQInteger OgreBufferUserData::OgreBufferCompare(HSQUIRRELVM vm){
        /*const OgreBufferData* first;
        const OgreBufferData* second;

        ScriptUtils::_debugStack(vm);
        SCRIPT_ASSERT_RESULT(readOgreBufferFromUserData(vm, -2, &first));
        SCRIPT_CHECK_RESULT(readOgreBufferFromUserData(vm, -1, &second));

        if(*first == *second){
            sq_pushinteger(vm, 0);
        }else{
            sq_pushinteger(vm, 2);
        }*/
        return 1;
    }

    void OgreBufferUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, OgreBufferToString, "_tostring");
        ScriptUtils::addFunction(vm, OgreBufferCompare, "_cmp");
        /**SQFunction
        @name upload
        @desc Write data into this buffer, replacing its contents. Unlike mapping,
        this can safely be called any number of times per frame. Immutable buffers
        (which includes all index buffers) cannot be uploaded to.
        @param1:DataBlob: The new data. Its size must be a whole number of elements.
        @param2:Integer: Element to start writing at. Defaults to 0.
        */
        ScriptUtils::addFunction(vm, upload, "upload", -2, ".xi");
        /**SQFunction
        @name getNumElements
        @desc Number of elements (vertices or indices) this buffer holds.
        @returns:Integer
        */
        ScriptUtils::addFunction(vm, getNumElements, "getNumElements");
        /**SQFunction
        @name getBytesPerElement
        @desc Size in bytes of a single element of this buffer.
        @returns:Integer
        */
        ScriptUtils::addFunction(vm, getBytesPerElement, "getBytesPerElement");

        sq_resetobject(&OgreBufferDelegateTableObject);
        sq_getstackobj(vm, -1, &OgreBufferDelegateTableObject);
        sq_addref(vm, &OgreBufferDelegateTableObject);
        sq_pop(vm, 1);
    }

    SQInteger OgreBufferUserData::OgreBufferToString(HSQUIRRELVM vm){
        OgreBufferData outData;
        SCRIPT_ASSERT_RESULT(readOgreBufferFromUserData(vm, 1, &outData));

        //TODO try and de-reference the pointer here to add more description.
        std::ostringstream stream;
        stream << (outData.bufType == OgreBufferType::vertexBuffer ? "VertexBuffer" : "IndexBuffer");
        sq_pushstring(vm, _SC(stream.str().c_str()), -1);

        return 1;
    }

    void OgreBufferUserData::setupConstants(HSQUIRRELVM vm){
        /**SQConstant
        @name _GPU_PROG_TYPE_VERTEX
        @desc Vertex program.
        */
        //ScriptUtils::declareConstant(vm, "_GPU_PROG_TYPE_VERTEX", (SQInteger)Ogre::GPT_VERTEX_PROGRAM);
    }
}
