#pragma once

#include "Job.h"

namespace AV{
    class RecipeData;

    class RecipeOgreMeshJob : public Job{
    public:
        RecipeOgreMeshJob(RecipeData *data);

        void process();
        void finish();

    private:
        RecipeData *_data;

        void _processFile();
    };
};
