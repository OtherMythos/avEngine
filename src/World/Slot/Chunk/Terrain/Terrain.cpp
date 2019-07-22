#include "Terrain.h"

#include "terra/Terra.h"

#include <Ogre.h>

namespace AV{
    Terrain::Terrain(){

        Ogre::Root* root = Ogre::Root::getSingletonPtr();
        Ogre::SceneManager* mgr = root->getSceneManager("Scene Manager");

        mTerra = new Ogre::Terra( Ogre::Id::generateNewId<Ogre::MovableObject>(),
                                        &mgr->_getEntityMemoryManager( Ogre::SCENE_STATIC ),
                                        mgr, 0, Ogre::Root::getSingletonPtr()->getCompositorManager2(),
                                        mgr->getCameras()[0]);

        mTerra->setCastShadows( false );
        //mTerra->load( "Heightmap.png", Ogre::Vector3( 64.0f, 1024.0f * 0.5f, 64.0f ), Ogre::Vector3( 50.0f, 50.0f, 50.0f ) );
        mTerra->load( "Heightmap.png", Ogre::Vector3( 0, 0, 0 ), Ogre::Vector3( 200.0f, 50.0f, 200.0f ) );

        Ogre::HlmsDatablock *datablock = root->getHlmsManager()->getDatablock( "TerraExampleMaterial" );
        mTerra->setDatablock( datablock );

        Ogre::SceneNode *rootNode = mgr->getRootSceneNode( Ogre::SCENE_STATIC );
        Ogre::SceneNode *sceneNode = rootNode->createChildSceneNode( Ogre::SCENE_STATIC );
        sceneNode->setVisible(true);
        sceneNode->attachObject( mTerra );

        /*Ogre::SceneNode *node = rootNode->createChildSceneNode(Ogre::SCENE_DYNAMIC);
        Ogre::Item *item = mgr->createItem("cube", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_DYNAMIC);
        node->attachObject((Ogre::MovableObject*)item);*/

    }

    Terrain::~Terrain(){

    }

    void Terrain::update(){
        mTerra->update( Ogre::Vector3(0, 0, 0), 0);
    }
}
