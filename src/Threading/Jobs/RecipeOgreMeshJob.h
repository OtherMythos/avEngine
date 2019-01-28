#pragma once

#include "Job.h"
#include "World/Slot/Recipe/OgreMeshRecipeData.h"


namespace AV{
    struct RecipeData;

    class RecipeOgreMeshJob : public Job{
    public:
        RecipeOgreMeshJob(RecipeData *data);

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
