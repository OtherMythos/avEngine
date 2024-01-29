#include "RecipeNavMeshJob.h"

#include "Logger/Log.h"
#include "World/Slot/Recipe/RecipeData.h"
#include "System/SystemSetup/SystemSettings.h"

#include "World/Nav/Parser/DetourMeshBinaryParser.h"
#include <string>

#include "System/Util/PathUtils.h"

#include "DetourNavMesh.h"
#ifndef _WIN32
    #include <dirent.h>
#endif
#include <regex>

namespace AV{
    RecipeNavMeshJob::RecipeNavMeshJob(RecipeData *data, uint32 numDefinedMeshes)
    : mData(data),
    mNumDefinedMeshes(numDefinedMeshes) {

    }

    void RecipeNavMeshJob::process(){
        //AV_INFO("Starting ogre recipe job! {}", mData->coord);

        _processFile();
    }

    void RecipeNavMeshJob::finish(){
        //AV_INFO("Finishing ogre recipe job! {}", mData->coord);

        mData->jobDoneCounter++;
    }

    void RecipeNavMeshJob::_getNavMeshId(const char* n, int* outNavId){
        const std::string name(n);
        size_t found = name.find_first_of("-");
        //Stringent checks here because if this is called it should have passed the regex.
        assert(found != std::string::npos);
        std::string first = name.substr(0, found);
        *outNavId = std::stoi(first);
    }

    bool RecipeNavMeshJob::_processFile(){
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if(!SystemSettings::isMapsDirectoryViable()) {
            AV_ERROR("There was an error processing nav mesh recipe job {}. The maps directory isn't viable.", mData->coord);
            return false;
        }
        //In this case we would never be able to read anything.
        if(mNumDefinedMeshes == 0) return true;

        if(!mData->navMeshData){
            mData->navMeshData = new std::vector<NavMeshTileData>();
        }
        const std::string basePath = SystemSettings::getMapsDirectory() + "/" + mData->coord.getFilePath() + "/nav/";
        DetourMeshBinaryParser binaryParser;

        //TODO Cross platform, switch to the c++17 filesystem api.
        #ifndef _WIN32
        DIR *d = opendir(basePath.c_str());
        size_t path_len = basePath.length();
        int r = -1;

        if (d){
            struct dirent *p;
            r = 0;
            while (!r && (p=readdir(d))){
                int r2 = -1;
                char *buf;
                size_t len;

                if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")){
                    continue;
                }

                static const std::regex floatRegex("^\\d+-\\d+.nav$");
                if(std::regex_match(p->d_name, floatRegex)){
                    //Try to load this file.
                    int navId = 0;
                    _getNavMeshId(p->d_name, &navId);
                    //There might have been problems reading the defined nav meshes from the json file or something.
                    if(navId >= mNumDefinedMeshes) continue;

                    DetourMeshBinaryParser::TileData outData;
                    bool success = binaryParser.parseTile(basePath + p->d_name, &outData);
                    if(success){
                        mData->navMeshData->push_back({
                            outData.tileData,
                            outData.dataSize,
                            0,
                            outData.x,
                            outData.y,
                            navId
                        });
                    }
                }
            }

            closedir(d);
        }
        #endif

        return true;
    }
}
