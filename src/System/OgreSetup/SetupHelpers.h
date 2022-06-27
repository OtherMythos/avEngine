#pragma once

#include "System/EngineFlags.h"
#include "OgreConfigFile.h"
#include "OgreResourceGroupManager.h"

#include "Logger/Log.h"
#include "filesystem/path.h"

namespace AV{

    /**
    General helper functions for system setup.
    */
    class SetupHelpers{
    public:
        static void addResourceLocation(const std::string& path, const std::string& groupName, const filesystem::path& relativePath){
            filesystem::path valuePath(path);
            filesystem::path resolvedPath;
            if(!valuePath.is_absolute()){
                //If an absolute path was not provided determine an absolute.
                resolvedPath = relativePath / valuePath;
                if(!resolvedPath.exists() || !resolvedPath.is_directory()) return;

                resolvedPath = resolvedPath.make_absolute();
            }else{
                resolvedPath = valuePath;
                if(!resolvedPath.exists() || !resolvedPath.is_directory()) return;
            }

            //Right now use filesystem for everything. In future if I add others I can sort this out.
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(resolvedPath.str(), "FileSystem", groupName);
            AV_INFO("Adding {} to {}", resolvedPath.str(), groupName);
        }

        static void parseOgreResourcesFile(const Ogre::String& path){
            AV_INFO("Parsing OgreResourceFile at path {}", path);

            Ogre::ConfigFile cf;
            cf.load(path);

            Ogre::String name, locType;
            Ogre::ConfigFile::SectionIterator secIt = cf.getSectionIterator();

            filesystem::path relativePath(SystemSettings::getOgreResourceFilePath());
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

                    SetupHelpers::addResourceLocation(name, groupName, relativePath);
                }
            }
        }

    };
}
