#pragma once

#include "Job.h"
#include "System/EnginePrerequisites.h"

namespace AV{
    struct RecipeData;

    class RecipeNavMeshJob : public Job{
    public:
        RecipeNavMeshJob(RecipeData *data, uint32 numDefinedMeshes);

        void process();
        void finish();

    private:
        RecipeData *mData;
        uint32 mNumDefinedMeshes;

        bool _processFile();

        void _getNavMeshId(const char* name, int* outNavId);
    };
};
