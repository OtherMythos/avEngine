#include "VideoUserData.h"

#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/TextureUserData.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Scripting/ScriptObjectTypeTags.h"
#include "Video/VideoPlayback.h"

#include <exception>

namespace AV{
    SQObject VideoUserData::videoDelegateTableObject;

    void VideoUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, advance, "advance", 2, ".n");
        ScriptUtils::addFunction(vm, rewind, "rewind", 1, ".");
        ScriptUtils::addFunction(vm, hasEnded, "hasEnded", 1, ".");
        ScriptUtils::addFunction(vm, getFrameRate, "getFrameRate", 1, ".");
        ScriptUtils::addFunction(vm, getPixelAspectRatio, "getPixelAspectRatio", 1, ".");
        ScriptUtils::addFunction(vm, getTexture, "getTexture", 1, ".");
        ScriptUtils::addFunction(vm, getLastFrameTimings, "getLastFrameTimings", 1, ".");

        sq_resetobject(&videoDelegateTableObject);
        sq_getstackobj(vm, -1, &videoDelegateTableObject);
        sq_addref(vm, &videoDelegateTableObject);
        sq_pop(vm, 1);
    }

    void VideoUserData::videoToUserData(HSQUIRRELVM vm, std::unique_ptr<VideoPlayback> video){
        auto** pointer = static_cast<VideoPlayback**>(sq_newuserdata(vm, sizeof(VideoPlayback*)));
        *pointer = video.release();

        sq_pushobject(vm, videoDelegateTableObject);
        sq_setdelegate(vm, -2);
        sq_settypetag(vm, -1, VideoTypeTag);
        sq_setreleasehook(vm, -1, videoReleaseHook);
    }

    VideoPlayback* VideoUserData::_readVideoFromUserData(HSQUIRRELVM vm){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, 1, &pointer, &typeTag)) || typeTag != VideoTypeTag){
            sq_throwerror(vm, "Expected an MPEG-1 video.");
            return nullptr;
        }
        return *static_cast<VideoPlayback**>(pointer);
    }

    SQInteger VideoUserData::videoReleaseHook(SQUserPointer p, SQInteger size){
        delete *static_cast<VideoPlayback**>(p);
        return 0;
    }

    SQInteger VideoUserData::advance(HSQUIRRELVM vm){
        auto* video = _readVideoFromUserData(vm);
        if(!video) return SQ_ERROR;

        SQFloat seconds;
        sq_getfloat(vm, 2, &seconds);
        try{
            sq_pushbool(vm, video->advance(seconds));
            return 1;
        }catch(const std::exception& error){
            return sq_throwerror(vm, error.what());
        }
    }

    SQInteger VideoUserData::rewind(HSQUIRRELVM vm){
        auto* video = _readVideoFromUserData(vm);
        if(!video) return SQ_ERROR;

        try{
            video->rewind();
            return 0;
        }catch(const std::exception& error){
            return sq_throwerror(vm, error.what());
        }
    }

    SQInteger VideoUserData::hasEnded(HSQUIRRELVM vm){
        auto* video = _readVideoFromUserData(vm);
        if(!video) return SQ_ERROR;
        sq_pushbool(vm, video->hasEnded());
        return 1;
    }

    SQInteger VideoUserData::getFrameRate(HSQUIRRELVM vm){
        auto* video = _readVideoFromUserData(vm);
        if(!video) return SQ_ERROR;
        sq_pushfloat(vm, video->getFrameRate());
        return 1;
    }

    SQInteger VideoUserData::getPixelAspectRatio(HSQUIRRELVM vm){
        auto* video = _readVideoFromUserData(vm);
        if(!video) return SQ_ERROR;
        sq_pushfloat(vm, video->getPixelAspectRatio());
        return 1;
    }

    SQInteger VideoUserData::getTexture(HSQUIRRELVM vm){
        auto* video = _readVideoFromUserData(vm);
        if(!video) return SQ_ERROR;
        TextureUserData::textureToUserData(vm, video->getTexture(), false);
        return 1;
    }

    SQInteger VideoUserData::getLastFrameTimings(HSQUIRRELVM vm){
        auto* video = _readVideoFromUserData(vm);
        if(!video) return SQ_ERROR;

        const auto timings = video->getLastFrameTimings();
        sq_newtable(vm);
        sq_pushstring(vm, "decodeMs", -1);
        sq_pushfloat(vm, timings.decodeMs);
        sq_newslot(vm, -3, false);
        sq_pushstring(vm, "uploadMs", -1);
        sq_pushfloat(vm, timings.uploadMs);
        sq_newslot(vm, -3, false);
        return 1;
    }
}
