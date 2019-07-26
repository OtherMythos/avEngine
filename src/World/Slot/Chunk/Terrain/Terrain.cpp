#include "Terrain.h"

#include "terra/Terra.h"

#include "System/SystemSetup/SystemSettings.h"

#include <Ogre.h>

namespace AV{
    Terrain::Terrain(Ogre::SceneNode* n){

        Ogre::Root* root = Ogre::Root::getSingletonPtr();
        Ogre::SceneManager* mgr = root->getSceneManager("Scene Manager");

        mTerra = new Ogre::Terra( Ogre::Id::generateNewId<Ogre::MovableObject>(),
                                        &mgr->_getEntityMemoryManager( Ogre::SCENE_STATIC ),
                                        mgr, 0, Ogre::Root::getSingletonPtr()->getCompositorManager2(),
                                        mgr->getCameras()[0]);

        mTerra->setCastShadows( false );

        Ogre::Vector3 nPos = n->getPosition();
        int slotSize = SystemSettings::getWorldSlotSize();
        mTerra->load( "Heightmap.png", Ogre::Vector3(nPos.x, 0, nPos.z), Ogre::Vector3(slotSize, slotSize, slotSize));

        Ogre::HlmsDatablock *datablock = root->getHlmsManager()->getDatablock( "TerraExampleMaterial" );
        mTerra->setDatablock( datablock );

        n->attachObject( mTerra );
    }

    Terrain::~Terrain(){

    }

    void Terrain::update(){
        //From the looks of it, the second value isn't used for anything other than determining whether the shadow map should be updated.
        //It's updated once every frame if the value is set to 0.
        //Setting it to 1 means the shadow map should only update once on creation, and I guess you could dynamically adjust this value depending on when you want to update it.
        mTerra->update( Ogre::Vector3( -1, -1, -1 ).normalisedCopy(), 1);
    }
}
