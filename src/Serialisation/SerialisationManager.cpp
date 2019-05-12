#include "SerialisationManager.h"

#include "Logger/Log.h"
#include "System/SystemSetup/SystemSettings.h"

#include "rapidjson/document.h"
#include <rapidjson/filereadstream.h>
#include "rapidjson/filewritestream.h"
#include <rapidjson/writer.h>

#include "filesystem/path.h"
#include <cstdio>

#ifndef _WIN32
    #include <sys/types.h>
    #include <dirent.h>
#endif

namespace AV{
    const SerialisationManager::SaveInfoData SerialisationManager::SaveInfoData::DEFAULT;
    
    SerialisationManager::SerialisationManager(){
        scanForSaves();
    }

    SerialisationManager::~SerialisationManager(){

    }

    //Some code found on stack overflow.
    //In future I'm going to be using the std::filesystem layer, and this can be cleaned up, because it won't work on Windows.
    int remove_directory(const char *path){
        DIR *d = opendir(path);
        size_t path_len = strlen(path);
        int r = -1;

        if (d){
            struct dirent *p;
            r = 0;
            while (!r && (p=readdir(d))){
                int r2 = -1;
                char *buf;
                size_t len;

                /* Skip the names "." and ".." as we don't want to recurse on them. */
                if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")){
                    continue;
                }

                len = path_len + strlen(p->d_name) + 2;
                buf = (char*)malloc(len);
                if(buf){
                    struct stat statbuf;

                    snprintf(buf, len, "%s/%s", path, p->d_name);
                    if (!stat(buf, &statbuf)){
                        if (S_ISDIR(statbuf.st_mode)){
                            r2 = remove_directory(buf);
                        }
                        else{
                            r2 = unlink(buf);
                        }
                    }

                    free(buf);
                }

                r = r2;
            }

            closedir(d);
        }
        if (!r){
            r = rmdir(path);
        }
        return r;
    }
    
    void SerialisationManager::prepareSaveDirectory(const SaveHandle &handle){
        filesystem::path directoryPath(handle.determineSaveDirectory());
        
        if(directoryPath.exists()){
            AV_INFO("Overriding existing save {}", handle.saveName);
            remove_directory(directoryPath.str().c_str());
        }else AV_INFO("Creating new save directory {}", handle.saveName);
        
        filesystem::create_directory(directoryPath);
        writeDataToSaveFile(handle, SaveInfoData::DEFAULT);
    }
    
    void SerialisationManager::clearAllSaves(){
        //Just get rid of everything in the saves directory.
        DIR* dirp = opendir(SystemSettings::getSaveDirectory().c_str());
        struct dirent * dp;
        while ((dp = readdir(dirp)) != NULL) {
            if(dp->d_name[0] == '.') continue;
            
            std::string dir(dp->d_name);
            filesystem::path p = filesystem::path(SystemSettings::getSaveDirectory()) / filesystem::path(dir);
            remove_directory(p.str().c_str());
        }
        closedir(dirp);
    }
    
    void SerialisationManager::getDataFromSaveFile(const SaveHandle& handle, SaveInfoData& data){
        FILE* fp = fopen(handle.determineSaveInfoFile().c_str(), "r"); // non-Windows use "r"
        char readBuffer[65536];
        rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        rapidjson::Document d;
        d.ParseStream(is);
        fclose(fp);

        if(d.HasParseError()){
            return;
        }

        data.playTime = d["playTime"].GetFloat();
        SlotPosition playerPos(
            d["world"]["playerPosition"][0].GetInt(),
            d["world"]["playerPosition"][1].GetInt(),
            Ogre::Vector3(
                d["world"]["playerPosition"][2].GetFloat(),
                d["world"]["playerPosition"][3].GetFloat(),
                d["world"]["playerPosition"][4].GetFloat()
            )
        );
        data.playerPos = playerPos;
        data.mapName = d["world"]["mapName"].GetString();
        data.playerLoadRadius = d["world"]["playerLoadRadius"].GetInt();
    }
    
    void SerialisationManager::writeDataToSaveFile(const SaveHandle& handle, const SaveInfoData& data){
        rapidjson::Document d;
        d.SetObject();
        rapidjson::Document::AllocatorType& allocator = d.GetAllocator();

        d.AddMember("engineVersion", "0.1", allocator);
        
        rapidjson::Value textPart;
        textPart.SetString(handle.saveName.c_str(), allocator);
        d.AddMember("saveName", textPart, allocator);
        
        d.AddMember("playTime", data.playTime, allocator);
        
        {
            rapidjson::Value worldObj(rapidjson::kObjectType);
            
            rapidjson::Value playerPosArray(rapidjson::kArrayType);
            playerPosArray
                .PushBack(data.playerPos.chunkX(), allocator)
                .PushBack(data.playerPos.chunkY(), allocator)
                .PushBack(data.playerPos.position().x, allocator)
                .PushBack(data.playerPos.position().y, allocator)
                .PushBack(data.playerPos.position().z, allocator);
            worldObj.AddMember("playerPosition", playerPosArray, allocator);
            
            rapidjson::Value mapName;
            mapName.SetString(data.mapName.c_str(), allocator);
            worldObj.AddMember("mapName", mapName, allocator);
            worldObj.AddMember("playerLoadRadius", data.playerLoadRadius, allocator);
            
            d.AddMember("world", worldObj, allocator);
        }


        FILE* fp = fopen(handle.determineSaveInfoFile().c_str(), "w"); // non-Windows use "w"
        char writeBuffer[65536];
        rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
        rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
        d.Accept(writer);
        fclose(fp);
    }

    void SerialisationManager::scanForSaves(){
        if(!SystemSettings::isSaveDirectoryViable()) return;

        mSaves.clear();

        //List the directories in this path.
        DIR* dirp = opendir(SystemSettings::getSaveDirectory().c_str());
        struct dirent * dp;
        while ((dp = readdir(dirp)) != NULL) {
            if(dp->d_name[0] == '.') continue;

            std::string dir(dp->d_name);
            _scanSaveDirectory(dir);
        }
        closedir(dirp);
    }

    void SerialisationManager::_scanSaveDirectory(const std::string& dirName){
        filesystem::path p = filesystem::path(SystemSettings::getSaveDirectory()) / filesystem::path(dirName);
        AV_INFO("Checking save directory {}", p);

        filesystem::path avSaveFile(p / filesystem::path("saveInfo.avSave"));
        if(avSaveFile.exists() && avSaveFile.is_file()){
            _processSaveInfo(avSaveFile.str());
        }
    }

    void SerialisationManager::_processSaveInfo(const std::string& filePath){
        FILE* fp = fopen(filePath.c_str(), "r"); // non-Windows use "r"
        char readBuffer[65536];
        rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        rapidjson::Document d;
        d.ParseStream(is);
        fclose(fp);

        if(d.HasParseError()){
            AV_ERROR("There was an error parsing the save file at path {}", filePath);
            return;
        }

        //Simple stuff right now.
        SaveHandle handle;
        handle.saveName = d["saveName"].GetString();
        mSaves.push_back(handle);

    }
}
