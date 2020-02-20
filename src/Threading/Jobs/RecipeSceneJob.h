#pragma once

#include "Job.h"


namespace AV{
    struct RecipeData;

    class RecipeSceneJob : public Job{
    public:
        RecipeSceneJob(RecipeData *data);

        void process();
        void finish();

    private:
        RecipeData *mData;

        /**
         Internal method to process the scene files.

         @return
         Whether or not the process was successful.
         */
        bool _processFile();
    };
};
