#pragma once

#include "Job.h"

namespace AV{
    struct RecipeData;

    class RecipePhysicsBodiesJob : public Job{
    public:
        RecipePhysicsBodiesJob(RecipeData *data);

        void process();
        void finish();

    private:
        RecipeData *_data;

        /**
         Internal method to process the meshes file.

         @return
         Whether or not the process was successful.
         */
        bool _processFile();
    };
};
