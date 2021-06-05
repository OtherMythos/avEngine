#include "GraphicsNamespace.h"

#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreTextureGpuManager.h"

#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/TextureUserData.h"

namespace AV{

    SQInteger GraphicsNamespace::createTexture(HSQUIRRELVM vm){
        const SQChar* workspaceName;
        sq_getstring(vm, 2, &workspaceName);

        Ogre::TextureGpu* tex = Ogre::Root::getSingletonPtr()->getRenderSystem()->getTextureGpuManager()
            ->createTexture(workspaceName, Ogre::GpuPageOutStrategy::Discard, Ogre::TextureFlags::RenderToTexture, Ogre::TextureTypes::Type2D);

        //tex->setResolution(100u, 100u);
        tex->setPixelFormat(Ogre::PFG_RGBA32_FLOAT);
        //tex->scheduleTransitionTo(Ogre::GpuResidency::Resident);
        TextureUserData::textureToUserData(vm, tex, true);

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
    }
}
