#include "GraphicsNamespace.h"

#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreTextureGpuManager.h"
#include "OgreResourceGroupManager.h"
#include "OgreMeshManager2.h"

#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/TextureUserData.h"

namespace AV{

    SQInteger GraphicsNamespace::createTexture(HSQUIRRELVM vm){
        const SQChar* textureName;
        sq_getstring(vm, 2, &textureName);

        Ogre::TextureGpuManager* manager = Ogre::Root::getSingletonPtr()->getRenderSystem()->getTextureGpuManager();

        Ogre::TextureGpu* tex = 0;
        try{
            tex = manager->createTexture(textureName, Ogre::GpuPageOutStrategy::Discard, Ogre::TextureFlags::RenderToTexture, Ogre::TextureTypes::Type2D);
        }catch(Ogre::Exception e){
            return sq_throwerror(vm, "Texture with name already exists.");
        }

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
    }
}
