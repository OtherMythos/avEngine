#pragma once

#include "Mesh/OgreMeshManager.h"

namespace AV{
    struct OgreMeshComponent {
        OgreMeshComponent(OgreMeshManager::OgreMeshPtr m)
            : mesh(m) {}

        ~OgreMeshComponent(){
            mesh.reset();
        }

        OgreMeshManager::OgreMeshPtr mesh;
    };
}
