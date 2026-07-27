#pragma once

#include "OgreResource.h"

namespace AV{

    /**
    Loads wavefront obj files as v2 Ogre meshes.
    Meshes are registered with the regular MeshManager as manual resources,
    so once registered a mesh named 'something.obj' can be used anywhere a
    regular .mesh file could be, for instance to create an item.
    */
    class ObjMeshLoader : public Ogre::ManualResourceLoader{
    public:
        /** The shared loader instance, to be registered with OgreMeshManager for the .obj extension. */
        static ObjMeshLoader& getLoader();

        /** @see ManualResourceLoader::loadResource. Parses the obj file and populates the mesh. */
        void loadResource(Ogre::Resource* resource);
    };
}
