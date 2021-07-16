#pragma once

#include "Job.h"


namespace AV{
    struct RecipeData;

    class RecipeNavMeshJob : public Job{
    public:
        RecipeNavMeshJob(RecipeData *data);

        void process();
        void finish();

    private:
        RecipeData *mData;

        bool _processFile();

        void _getNavMeshId(const char* name, int* outNavId);
    };
};
