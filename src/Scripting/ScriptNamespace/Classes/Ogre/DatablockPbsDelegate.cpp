#include "DatablockPbsDelegate.h"

#include "DatablockUserData.h"
#include "OgreHlmsPbsDatablock.h"
#include "OgreTextureManager.h"

namespace AV{
    void DatablockPbsDelegate::setupTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 2);

        ScriptUtils::addFunction(vm, setDiffuse, "setDiffuse");
        ScriptUtils::addFunction(vm, setMetalness, "setMetalness");
    }

    SQInteger DatablockPbsDelegate::setDiffuse(HSQUIRRELVM vm){
        SQFloat x, y, z;
        sq_getfloat(vm, -1, &z);
        sq_getfloat(vm, -2, &y);
        sq_getfloat(vm, -3, &x);

        Ogre::HlmsDatablock* db = DatablockUserData::getPtrFromUserData(vm, -4);
        assert(db->mType == Ogre::HLMS_PBS);

        Ogre::HlmsPbsDatablock* b = (Ogre::HlmsPbsDatablock*)db;
        b->setDiffuse(Ogre::Vector3(x, y, z));

        return 0;
    }

    SQInteger DatablockPbsDelegate::setMetalness(HSQUIRRELVM vm){
        SQFloat metalness;
        sq_getfloat(vm, -1, &metalness);

        Ogre::HlmsDatablock* db = DatablockUserData::getPtrFromUserData(vm, -2);
        assert(db->mType == Ogre::HLMS_PBS);

        Ogre::HlmsPbsDatablock* b = (Ogre::HlmsPbsDatablock*)db;
        b->setMetalness(metalness);

        return 0;
    }


}
