#ifdef FLIGHT_RECORDER

#pragma once

#include <cstdint>

namespace AV{

    /**
    Rolling per stage timings for one recorded frame, in microseconds.

    The recorder's cost lands squarely on the main thread between render commands being
    issued and the frame being presented, so knowing which stage owns it is the difference
    between tuning the right thing and guessing. Exposed through GET /api/recorder.
    */
    struct RecorderStats{
        //Exponential moving averages, in microseconds.
        double readbackUs = 0.0;
        //Split of readbackUs on the synchronous path.
        double downloadUs = 0.0;
        double unpackUs = 0.0;
        double convertUs = 0.0;
        double hashUs = 0.0;
        double totalUs = 0.0;
        //Worst total seen since the last reset.
        double peakTotalUs = 0.0;
        uint64_t framesRecorded = 0;
        uint64_t framesSkipped = 0;

        //Pixels read back per frame, so a cost can be judged per pixel rather than in the abstract.
        uint32_t sourceWidth = 0;
        uint32_t sourceHeight = 0;
        uint32_t storedWidth = 0;
        uint32_t storedHeight = 0;

        void accumulateSplit(double download, double unpack){
            const double weight = framesRecorded == 0 ? 1.0 : 0.05;
            downloadUs += (download - downloadUs) * weight;
            unpackUs += (unpack - unpackUs) * weight;
        }

        void accumulate(double readback, double convert, double hash){
            const double total = readback + convert + hash;
            //Smoothed rather than instantaneous: a single frame's timing is dominated by
            //scheduling noise, and the question being asked is what it costs on average.
            const double weight = framesRecorded == 0 ? 1.0 : 0.05;
            readbackUs += (readback - readbackUs) * weight;
            convertUs += (convert - convertUs) * weight;
            hashUs += (hash - hashUs) * weight;
            totalUs += (total - totalUs) * weight;
            if(total > peakTotalUs) peakTotalUs = total;
            framesRecorded++;
        }

        void reset(){
            *this = RecorderStats();
        }
    };
}

#endif
