#include "GraphicsNamespace.h"

#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreTextureGpuManager.h"

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
    }
}
