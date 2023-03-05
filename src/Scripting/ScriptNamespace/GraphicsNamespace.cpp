#include "GraphicsNamespace.h"

#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreTextureGpuManager.h"
#include "OgreResourceGroupManager.h"
#include "OgreMeshManager2.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "Vao/OgreVaoManager.h"

#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/TextureUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/GPUProgramUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/MaterialUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/MeshUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/OgreBufferUserData.h"

#include <sqstdblob.h>

namespace AV{

    SQInteger GraphicsNamespace::createTexture(HSQUIRRELVM vm){
        const SQChar* textureName;
        sq_getstring(vm, 2, &textureName);

        Ogre::TextureGpuManager* manager = Ogre::Root::getSingletonPtr()->getRenderSystem()->getTextureGpuManager();

        Ogre::TextureGpu* tex = 0;
        WRAP_OGRE_ERROR(
            tex = manager->createTexture(textureName, Ogre::GpuPageOutStrategy::Discard, Ogre::TextureFlags::RenderToTexture, Ogre::TextureTypes::Type2D);
        )

        tex->setPixelFormat(Ogre::PFG_RGBA32_FLOAT);
        TextureUserData::textureToUserData(vm, tex, true);

        return 1;
    }

    SQInteger GraphicsNamespace::createOrRetreiveTexture(HSQUIRRELVM vm){
        const SQChar* textureName;
        sq_getstring(vm, 2, &textureName);

        Ogre::TextureGpuManager* manager = Ogre::Root::getSingletonPtr()->getRenderSystem()->getTextureGpuManager();

        Ogre::TextureGpu* tex = 0;
        tex = manager->createOrRetrieveTexture(textureName, Ogre::GpuPageOutStrategy::Discard, Ogre::TextureFlags::RenderToTexture, Ogre::TextureTypes::Type2D);

        TextureUserData::textureToUserData(vm, tex, true);

        return 1;
    }

    SQInteger GraphicsNamespace::destroyTexture(HSQUIRRELVM vm){
        Ogre::TextureGpuManager* manager = Ogre::Root::getSingletonPtr()->getRenderSystem()->getTextureGpuManager();

        Ogre::TextureGpu* outTex;
        bool userOwned, valid;
        SCRIPT_ASSERT_RESULT(TextureUserData::readTextureFromUserData(vm, 2, &outTex, &userOwned, &valid));
        if(!valid){
            return sq_throwerror(vm, "Texture is invalid.");
        }
        if(!userOwned){
            return sq_throwerror(vm, "This texture cannot be destroyed.");
        }

        manager->destroyTexture(outTex);

        return 0;
    }

    SQInteger GraphicsNamespace::getLoadedMeshes(HSQUIRRELVM vm){
        Ogre::ResourceGroupManager* man = Ogre::ResourceGroupManager::getSingletonPtr();
        Ogre::StringVector groupsVec = man->getResourceGroups();

        sq_newarray(vm, 0);
        for(const Ogre::String& group : groupsVec){
            Ogre::FileInfoListPtr vec = man->findResourceFileInfo(group, "*.mesh");

            Ogre::FileInfoList::const_iterator itEntry = vec->begin();
            Ogre::FileInfoList::const_iterator enEntry = vec->end();

            while(itEntry != enEntry){
                const Ogre::String& en = itEntry->basename;

                sq_pushstring(vm, en.c_str(), -1);
                sq_arrayinsert(vm, -2, 0);

                ++itEntry;
            }
        }

        return 1;
    }

    SQInteger GraphicsNamespace::getGpuProgramByName(HSQUIRRELVM vm){
        const SQChar* progName;
        sq_getstring(vm, 2, &progName);

        Ogre::HighLevelGpuProgramPtr prog = Ogre::HighLevelGpuProgramManager::getSingleton().getByName(progName);

        if(prog.isNull()){
            Ogre::String str("No GPU program found with name ");
            str += progName;
            return sq_throwerror(vm, str.c_str());
        }

        GPUProgramUserData::GPUProgramToUserData(vm, prog);

        return 1;
    }

    SQInteger GraphicsNamespace::getMaterialByName(HSQUIRRELVM vm){
        const SQChar* matName;
        sq_getstring(vm, 2, &matName);

        Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName(matName);

        if(mat.isNull()){
            Ogre::String str("No material found with name ");
            str += matName;
            return sq_throwerror(vm, str.c_str());
        }

        MaterialUserData::MaterialToUserData(vm, mat);

        return 1;
    }

    SQInteger GraphicsNamespace::getLoadedTextures(HSQUIRRELVM vm){
        Ogre::TextureGpuManager* manager = Ogre::Root::getSingletonPtr()->getRenderSystem()->getTextureGpuManager();
        const Ogre::TextureGpuManager::ResourceEntryMap& entries = manager->getEntries();

        Ogre::TextureGpuManager::ResourceEntryMap::const_iterator itEntry = entries.begin();
        Ogre::TextureGpuManager::ResourceEntryMap::const_iterator enEntry = entries.end();

        sq_newarray(vm, 0);
        while(itEntry != enEntry){
            const Ogre::TextureGpuManager::ResourceEntry &entry = itEntry->second;

            sq_pushstring(vm, entry.name.c_str(), -1);
            sq_arrayinsert(vm, -2, 0);

            ++itEntry;
        }

        const Ogre::StringVector groupVec = Ogre::ResourceGroupManager::getSingleton().getResourceGroups();

        for(const Ogre::String& groupName : groupVec){
            if(groupName == "Internal") continue;
            Ogre::ResourceGroupManager::LocationList list = Ogre::ResourceGroupManager::getSingleton().getResourceLocationList(groupName);

            Ogre::ResourceGroupManager::LocationList::const_iterator listitEntry = list.begin();
            Ogre::ResourceGroupManager::LocationList::const_iterator listenEntry = list.end();
            while(listitEntry != listenEntry){
                const Ogre::ResourceGroupManager::ResourceLocation *entry = *listitEntry;
                Ogre::FileInfoListPtr strings = entry->archive->listFileInfo();
                for(const Ogre::FileInfo& e : *strings){
                    if(
                       e.filename.find(".png") == Ogre::String::npos &&
                       e.filename.find(".jpg") == Ogre::String::npos &&
                       e.filename.find(".dds") == Ogre::String::npos &&
                       e.filename.find(".jpeg") == Ogre::String::npos
                    ) continue;

                    sq_pushstring(vm, e.filename.c_str(), -1);
                    sq_arrayinsert(vm, -2, 0);
                }

                ++listitEntry;
            }
        }

        return 1;
    }

    SQInteger GraphicsNamespace::createManualMesh(HSQUIRRELVM vm){
        const SQChar* meshName;
        sq_getstring(vm, 2, &meshName);

        Ogre::String groupName = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME;
        if(sq_gettop(vm) >= 2){
            const SQChar* groupName = 0;
            sq_getstring(vm, 2, &groupName);
            groupName = groupName;
        }

        Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual(meshName, groupName);
        MeshUserData::MeshToUserData(vm, mesh);

        return 1;
    }

    SQInteger GraphicsNamespace::createVertexBuffer(HSQUIRRELVM vm){

        Ogre::RenderSystem *renderSystem = Ogre::Root::getSingletonPtr()->getRenderSystem();
        Ogre::VaoManager *vaoManager = renderSystem->getVaoManager();

        SQInteger numVerts;
        sq_getinteger(vm, 3, &numVerts);
        SQInteger strideSize;
        sq_getinteger(vm, 4, &strideSize);

        SQUserPointer blobData = 0;
        if(SQ_FAILED(sqstd_getblob(vm, 5, &blobData))){
            return sq_throwerror(vm, "Unknown type passed as value.");
        }
        SQInteger blobSize = sqstd_getblobsize(vm, 5);

        float *vertices = reinterpret_cast<float*>( OGRE_MALLOC_SIMD(sizeof(float) * blobSize, Ogre::MEMCATEGORY_GEOMETRY ) );

        memcpy(vertices, static_cast<float*>(blobData), sizeof(float) * blobSize);

        Ogre::VertexBufferPacked *vertexBuffer = 0;
        try{
            Ogre::VertexElement2Vec vertexElements;
            vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_POSITION));
            vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES));

            vertexBuffer = vaoManager->createVertexBuffer(vertexElements, strideSize, Ogre::BT_DEFAULT, vertices, true);
        }catch(Ogre::Exception &e){
            vertexBuffer = 0;
            return sq_throwerror(vm, e.getFullDescription().c_str());
        }

        OgreBufferUserData::OgreBufferData data{vertexBuffer, OgreBufferUserData::OgreBufferType::vertexBuffer};
        OgreBufferUserData::OgreBufferToUserData(vm, &data);

        return 1;
    }

    /**SQNamespace
    @name _graphics
    @desc Exposes functions to access graphical functions, for instance textures.
    */
    void GraphicsNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name createTexture
        @desc Create a texture by name. Throws an error if that texture already exists.
        @param1:String: The name of the texture to create.
        */
        ScriptUtils::addFunction(vm, createTexture, "createTexture", 2, ".s");
        /**SQFunction
        @name createOrRetreiveTexture
        @desc Create a texture by name, unless it exists in which case return it.
        @param1:String: The name of the texture.
        */
        ScriptUtils::addFunction(vm, createOrRetreiveTexture, "createOrRetreiveTexture", 2, ".s");
        /**SQFunction
        @name destroyTexture
        @desc Destroy a texture by handle. Must be a user editable texture or an error will be thrown.
        @param1:String: The name of the texture to create.
        */
        ScriptUtils::addFunction(vm, destroyTexture, "destroyTexture", 2, ".u");
        /**SQFunction
        @name getLoadedTextures
        @desc Returns a list of every texture which is currently registered to the texture manager.
        @return: A list of textures loaded, containing strings.
        */
        ScriptUtils::addFunction(vm, getLoadedTextures, "getLoadedTextures");

        ScriptUtils::addFunction(vm, getLoadedMeshes, "getLoadedMeshes");
        /**SQFunction
        @name getGpuProgramByName
        @desc Obtain a handle to a GPUProgram by its name.
        @param1:String: The name identifier of the gpu program.
        @returns:The found GPU program. An error is thrown if nothing is found.
        */
        ScriptUtils::addFunction(vm, getGpuProgramByName, "getGpuProgramByName", 2, ".s");
        /**SQFunction
        @name createManualMesh
        @desc Create a mesh object to be populated through code, i.e not loaded from a mesh file.
        @param1:String: The resource name
        @param2:String: The resource group
        @returns: A mesh object
        */
        ScriptUtils::addFunction(vm, createManualMesh, "createManualMesh", 2, ".ss");
        /**SQFunction
        @name createVertexBuffer
        @desc Create a vertex buffer object.
        @param1:VertexElemVec: VertexElemVec to describe the packing of the vertices.
        @param2:Integer: Number of vertices.
        @param3:Integer: Stride size.
        @param4:DataBlob: Blob of data containing the vertice data.
        @returns: A populated vertex buffer.
        */
        ScriptUtils::addFunction(vm, createVertexBuffer, "createVertexBuffer", 5, ".uiix");
    }
}
