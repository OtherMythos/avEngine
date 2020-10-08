#pragma once

#include "Job.h"


namespace AV{
    struct RecipeData;

    /**
    Job for generic meta data related tasks on chunk load.
    This could include processing json or other meta file types.
    */
    class RecipeChunkMetaJob : public Job{
    public:
        RecipeChunkMetaJob(RecipeData *data);

        void process();
        void finish();

    private:
        RecipeData *mData;

        bool _processFile();
    };
};
