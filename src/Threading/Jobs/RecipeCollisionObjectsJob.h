#pragma once

#include "Job.h"

namespace AV{
    struct RecipeData;

    class RecipeCollisionObjectsJob : public Job{
    public:
        RecipeCollisionObjectsJob(RecipeData *data);

        void process();
        void finish();

    private:
        RecipeData *_data;
    };
};
