#include "EntityDeSerialisationJob.h"

#include "World/Entity/EntityManager.h"

#include "World/Entity/Components/PositionComponent.h"
#include "World/Entity/Logic/OgreMeshComponentLogic.h"
#include "World/Entity/Logic/ScriptComponentLogic.h"
#include "World/Entity/Logic/FundamentalLogic.h"

#include "World/Entity/Components/OgreMeshComponent.h"
#include "World/Entity/Components/ScriptComponent.h"
#include <OgreStringConverter.h>

#include "Logger/Log.h"

namespace AV{
    EntityDeSerialisationJob::EntityDeSerialisationJob(const SaveHandle& handle, std::atomic<int> *progressCounter, std::shared_ptr<EntityManager> manager, SerialiserStringStore *entityMeshStore, SerialiserStringStore *entityScriptStore)
        : mProgressCounter(progressCounter),
          mEntityManager(manager),
          mSaveHandle(handle),
          mEntityMeshStore(entityMeshStore),
          mEntityScriptStore(entityScriptStore){

    }

    void EntityDeSerialisationJob::process(){
        AV_INFO("Starting world entity deSerialisation job!");

        std::string line;
        std::ifstream file(mSaveHandle.determineEntitySerialisedFile());
        if (file.is_open()){
            while(!file.eof()){
                _beginEntity(file);
            }
        }
        
        file.close();
    }
    
    bool EntityDeSerialisationJob::_processPositionComponent(std::ifstream& file, eId& entity){
        std::string line;
        
        getline(file, line);
        if(line != "[Position]") return false;
        getline(file, line);
        int chunkX = Ogre::StringConverter::parseInt(line);
        getline(file, line);
        int chunkY = Ogre::StringConverter::parseInt(line);
        getline(file, line);
        Ogre::Vector3 pos = Ogre::StringConverter::parseVector3(line);
        getline(file, line);
        bool tracked = Ogre::StringConverter::parseBool(line);
        
        if(tracked)
            entity = mEntityManager->createEntityTracked(SlotPosition(chunkX, chunkY, pos));
        else
            entity = mEntityManager->createEntity(SlotPosition(chunkX, chunkY, pos));
        
        return true;
    }
    
    void EntityDeSerialisationJob::_beginEntity(std::ifstream& file){
        AV_INFO("Beginning entity");
        std::string line;
        
        eId entity;
        
        //Process the position component. This will also create the entity.
        if(!_processPositionComponent(file, entity)){
            //If the position component can't be created then the rest of the entity can't.
            _abortEntity(file);
        }
        
        //Assume at this point that the entity created is valid.
        while(getline(file, line)){
            if(line == "--") break;
            //line currently contains the component header, so that needs passing in.
            _iterateEntityComponents(entity, file, line);
        }
        AV_INFO("Loaded an entity");
    }
    
    void EntityDeSerialisationJob::_iterateEntityComponents(eId entity, std::ifstream& file, const std::string &line){
        if(line == "[OgreMesh]"){
            OgreMeshComponentLogic::deserialise(entity, file, mEntityMeshStore);
        }
        if(line == "[Script]"){
            ScriptComponentLogic::deserialise(entity, file, mEntityScriptStore);
        }
    }
    
    void EntityDeSerialisationJob::_abortEntity(std::ifstream& file){
        std::string line;
        while(getline(file, line)){
            if(line == "--") break;
        }
    }

    void EntityDeSerialisationJob::finish(){
        AV_INFO("Finishing world entity deSerialisation job!");

        (*mProgressCounter)++;
    }

}
