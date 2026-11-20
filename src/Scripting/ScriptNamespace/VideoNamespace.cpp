#include "VideoNamespace.h"

#include "Scripting/ScriptNamespace/Classes/Video/VideoUserData.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "System/Util/PathUtils.h"
#include "Video/VideoPlayback.h"

#include <exception>
#include <memory>

namespace AV{
    SQInteger VideoNamespace::openVideo(HSQUIRRELVM vm){
        const SQChar* path;
        sq_getstring(vm, 2, &path);

        try{
            std::string filePath;
            formatResToPathVM(vm, path, filePath);
            VideoUserData::videoToUserData(vm, std::make_unique<VideoPlayback>(filePath));
            return 1;
        }catch(const std::exception& error){
            return sq_throwerror(vm, error.what());
        }
    }

    /**SQNamespace
    @name _video
    @desc Opens MPEG-1 video files for playback into a texture.
    */
    void VideoNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name open
        @desc Open an MPEG-1 program stream and upload its first frame.
        @param1:path: Engine path to the video file.
        @returns A video instance.
        */
        ScriptUtils::addFunction(vm, openVideo, "open", 2, ".s");
    }
}
