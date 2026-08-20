#ifdef FLIGHT_RECORDER

#pragma once

#include <cstdint>
#include <cstddef>

namespace AV{

    /**
    Runtime configuration of the flight recorder, populated from the command line.
    */
    struct RecorderSettings{
        //How many rendered frames the circular buffer holds.
        uint32_t ringFrames = 100;
        //Resolution frames are downsampled to before entering the ring.
        uint32_t captureWidth = 480;
        uint32_t captureHeight = 270;
        //Record only every nth rendered frame. 1 records every frame.
        uint32_t everyNthFrame = 1;
        //How many squirrel call/return events the trace ring holds.
        uint32_t scriptEvents = 65536;
        /**
        Upper bound on the pixel data held by the ring. The ring evicts oldest first to stay
        under this even if that means holding fewer than ringFrames frames, so a project with
        an unusually large capture resolution cannot silently consume gigabytes.
        */
        size_t maxFrameBytes = 256 * 1024 * 1024;
    };
}

#endif
