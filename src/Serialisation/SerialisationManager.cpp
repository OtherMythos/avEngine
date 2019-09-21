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
#else
	#include <experimental/filesystem>
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
	#ifndef _WIN32
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
	#else
		//Windows was being a nob and I couldn't figure out how to use its api to recursively delete a directory.
		//So I'm using the c++ filesystem to do it!
		//I'm not supposed to be doing that yet because I wanted to wait for it to mature a bit before trying it, but I'll just do that now.
		//I don't use windows for regular development anyway so who cares.

		try {
			std::experimental::filesystem::remove_all(path);
		}
		catch (std::experimental::filesystem::filesystem_error& e) {
			const char* reason = e.what();
			AV_ERROR("Error removing directory {}, {}", path, reason);
		}
		//The number doesn't mean anything in this case.
		return 1;
	#endif
    }

    void SerialisationManager::prepareSaveDirectory(const SaveHandle &handle){
        //TODO I've realised this name conflicts with the SystemSettings SaveDirectory. Change that.
        filesystem::path directoryPath(handle.determineSaveDirectory());

        if(directoryPath.exists()){
            AV_INFO("Overriding existing save {}", handle.saveName);
            remove_directory(directoryPath.str().c_str());
        }else AV_INFO("Creating new save directory {}", handle.saveName);

        filesystem::create_directory(directoryPath);
        writeDataToSaveFile(handle, SaveInfoData::DEFAULT);
    }

    void SerialisationManager::clearAllSaves(){
        if(!SystemSettings::isSaveDirectoryViable()) return;

        //Just get rid of everything in the saves directory.
	#ifndef _WIN32
        DIR* dirp = opendir(SystemSettings::getSaveDirectory().c_str());
        struct dirent * dp;
        while ((dp = readdir(dirp)) != NULL) {
            if(dp->d_name[0] == '.') continue;

            std::string dir(dp->d_name);
            filesystem::path p = filesystem::path(SystemSettings::getSaveDirectory()) / filesystem::path(dir);
            remove_directory(p.str().c_str());
        }
        closedir(dirp);
	#else
        for (auto& p : std::experimental::filesystem::directory_iterator(SystemSettings::getSaveDirectory().c_str())) {
            const std::string & dirName = p.path().filename().string();

            std::experimental::filesystem::remove_all(dirName);
        }
	#endif

		//Just delete the entire save directory and re-create it.
		remove_directory(SystemSettings::getSaveDirectory().c_str());
		filesystem::create_directory(SystemSettings::getSaveDirectory().c_str());

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

        //data.playTime = d["playTime"].GetFloat();
        SlotPosition playerPos(
            d["world"]["playerPosition"][0].GetInt(),
            d["world"]["playerPosition"][1].GetInt(),
            Ogre::Vector3(
                d["world"]["playerPosition"][2].GetDouble(),
                d["world"]["playerPosition"][3].GetDouble(),
                d["world"]["playerPosition"][4].GetDouble()
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
#ifdef _WIN32
        std::string savePath = handle.determineSaveInfoFile();
        FILE* fp = fopen(savePath.c_str(), "wb");
#else
        FILE* fp = fopen(handle.determineSaveInfoFile().c_str(), "w"); // non-Windows use "w"
#endif
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
#ifndef _WIN32
        DIR* dirp = opendir(SystemSettings::getSaveDirectory().c_str());
        struct dirent * dp;
        while ((dp = readdir(dirp)) != NULL) {
            if(dp->d_name[0] == '.') continue;

			//TODO add a check as to whether this is actually a directory and not a file.
            std::string dir(dp->d_name);
            _scanSaveDirectory(dir);
        }
        closedir(dirp);
#else
		for(auto& p : std::experimental::filesystem::directory_iterator(SystemSettings::getSaveDirectory().c_str())) {
			if (!std::experimental::filesystem::is_directory(p.path())) continue;
			if (p.path().filename().string()[0] == '.') continue;

            const std::string& dirName = p.path().filename().string();
			_scanSaveDirectory(dirName);

		}
#endif
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
#ifdef _WIN32
        FILE* fp = fopen(filePath.c_str(), "rb");
#else
        FILE* fp = fopen(filePath.c_str(), "r"); // non-Windows use "r"
#endif
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
