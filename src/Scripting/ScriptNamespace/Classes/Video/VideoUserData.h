#pragma once

#include <squirrel.h>
#include <memory>

namespace AV{
    class VideoPlayback;

    class VideoUserData{
    public:
        VideoUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);
        static void videoToUserData(HSQUIRRELVM vm, std::unique_ptr<VideoPlayback> video);

    private:
        static SQObject videoDelegateTableObject;

        static SQInteger advance(HSQUIRRELVM vm);
        static SQInteger rewind(HSQUIRRELVM vm);
        static SQInteger hasEnded(HSQUIRRELVM vm);
        static SQInteger getFrameRate(HSQUIRRELVM vm);
        static SQInteger getPixelAspectRatio(HSQUIRRELVM vm);
        static SQInteger getTexture(HSQUIRRELVM vm);
        static SQInteger getLastFrameTimings(HSQUIRRELVM vm);

        static VideoPlayback* _readVideoFromUserData(HSQUIRRELVM vm);
        static SQInteger videoReleaseHook(SQUserPointer p, SQInteger size);
    };
}
