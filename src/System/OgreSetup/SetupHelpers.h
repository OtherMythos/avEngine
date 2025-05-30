#pragma once

#include "System/EngineFlags.h"
#include "OgreConfigFile.h"
#include "OgreResourceGroupManager.h"
#include "System/Util/FileSystemHelper.h"

#include "Logger/Log.h"
#include "System/FileSystem/FilePath.h"

namespace AV{

    /**
    General helper functions for system setup.
    */
    class SetupHelpers{
    public:
        static void addResourceLocation(const std::string& path, const std::string& groupName, const AV::FilePath& relativePath, std::string locationType = "FileSystem"){
            AV::FilePath valuePath(path);
            AV::FilePath resolvedPath;
            if(!valuePath.is_absolute()){
                //If an absolute path was not provided determine an absolute.
                resolvedPath = relativePath / valuePath;
            }else{
                resolvedPath = valuePath;
            }

            if(!resolvedPath.exists()){
                AV_WARN("Unable to add resource location {} as that path does not exist.", resolvedPath.str());
                return;
            }
            if(!resolvedPath.is_directory()){
                AV_WARN("Unable to add resource location {} as that path is not a directory.", resolvedPath.str());
                return;
            }

            resolvedPath = resolvedPath.make_absolute();

            //Right now use filesystem for everything. In future if I add others I can sort this out.
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(resolvedPath.str(), locationType, groupName);
            AV_INFO("Adding {} to {}", resolvedPath.str(), groupName);
        }

        static void parseOgreResourcesFile(const Ogre::String& path){
            AV_INFO("Parsing OgreResourceFile at path {}", path);

            Ogre::ConfigFile cf;
            FileSystemHelper::loadOgreConfigFile(cf, path);

            Ogre::String name, locType;
            Ogre::ConfigFile::SectionIterator secIt = cf.getSectionIterator();

            AV::FilePath relativePath(path);
            relativePath = relativePath.parent_path();

            while (secIt.hasMoreElements()){
                const std::string groupName = secIt.peekNextKey();
                Ogre::ConfigFile::SettingsMultiMap* settings = secIt.getNext();
                Ogre::ConfigFile::SettingsMultiMap::iterator it;

                if(!EngineFlags::resourceGroupValid(groupName)){
                    AV_ERROR("Skipping resource location {} as it conflicts with an engine reserved name.", groupName);
                    continue;
                }

                for (it = settings->begin(); it != settings->end(); ++it){
                    locType = it->first;
                    name = it->second;

                    #ifdef TARGET_ANDROID
                        if(locType == "FileSystem"){
                            locType = "APKFileSystem";
                        }
                    #endif
                    SetupHelpers::addResourceLocation(name, groupName, relativePath, locType);
                }
            }
        }

    };
}
