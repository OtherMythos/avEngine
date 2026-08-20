#ifdef FLIGHT_RECORDER

#include "RecorderNamespace.h"

#include "System/FlightRecorder/FlightRecorder.h"
#include "System/FlightRecorder/ScriptTrace.h"

namespace AV{

    SQInteger RecorderNamespace::capture(HSQUIRRELVM vm){
        std::string description;
        if(sq_gettop(vm) >= 2){
            const SQChar* value;
            sq_getstring(vm, 2, &value);
            description = value;
        }

        //Called from script, so the vm's stack is live here and the capture will carry a
        //real backtrace, unlike one triggered by the hotkey.
        FlightRecorder::capture(CaptureReason::Script, description);
        return 0;
    }

    SQInteger RecorderNamespace::mark(HSQUIRRELVM vm){
        const SQChar* tag;
        sq_getstring(vm, 2, &tag);

        ScriptTrace::mark(tag);
        return 0;
    }

    SQInteger RecorderNamespace::watch(HSQUIRRELVM vm){
        const SQChar* name;
        sq_getstring(vm, 2, &name);

        HSQOBJECT closure;
        sq_resetobject(&closure);
        sq_getstackobj(vm, 3, &closure);

        ScriptTrace::watch(name, closure);
        return 0;
    }

    SQInteger RecorderNamespace::unwatch(HSQUIRRELVM vm){
        const SQChar* name;
        sq_getstring(vm, 2, &name);

        sq_pushbool(vm, ScriptTrace::unwatch(name) ? SQTrue : SQFalse);
        return 1;
    }

    SQInteger RecorderNamespace::getWatches(HSQUIRRELVM vm){
        const std::vector<std::string> names = ScriptTrace::watchNames();

        sq_newarray(vm, 0);
        for(const std::string& name : names){
            sq_pushstring(vm, name.c_str(), name.size());
            sq_arrayappend(vm, -2);
        }
        return 1;
    }

    SQInteger RecorderNamespace::isEnabled(HSQUIRRELVM vm){
        sq_pushbool(vm, FlightRecorder::isRunning() ? SQTrue : SQFalse);
        return 1;
    }

    SQInteger RecorderNamespace::start(HSQUIRRELVM vm){
        FlightRecorder::start();
        return 0;
    }

    SQInteger RecorderNamespace::stop(HSQUIRRELVM vm){
        FlightRecorder::stop();
        return 0;
    }

    SQInteger RecorderNamespace::getFramesBuffered(HSQUIRRELVM vm){
        sq_pushinteger(vm, static_cast<SQInteger>(FlightRecorder::framesBuffered()));
        return 1;
    }

    SQInteger RecorderNamespace::getLastCapturePath(HSQUIRRELVM vm){
        const std::string& path = FlightRecorder::lastCapturePath();
        sq_pushstring(vm, path.c_str(), path.size());
        return 1;
    }

    /**SQNamespace
    @name _recorder
    @desc Drive the flight recorder, which keeps the last few seconds of rendered frames and
    script activity in a circular buffer and can dump them to disk for later diagnosis.
    Available when the engine was launched with --flightRecorder.
    */
    void RecorderNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name capture
        @param1:description: Optional text describing what looked wrong.
        @desc Dump the buffered frames and script activity to a capture directory. Because
        this runs while script code is executing, the capture also carries a live backtrace
        with local variables, which a capture triggered by the hotkey cannot have.
        */
        ScriptUtils::addFunction(vm, capture, "capture", -1, ".s");
        /**SQFunction
        @name mark
        @param1:tag: Text to attach to the frame currently being recorded.
        @desc Annotate this frame, so a moment of interest can be found in the timeline later.
        */
        ScriptUtils::addFunction(vm, mark, "mark", 2, ".s");
        /**SQFunction
        @name watch
        @param1:name: Name the value is recorded under.
        @param2:closure: Called once per frame; whatever it returns is recorded for that frame.
        @desc Sample a value every frame. Tables and arrays are expanded to a bounded depth.
        */
        ScriptUtils::addFunction(vm, watch, "watch", 3, ".sc");
        /**SQFunction
        @name unwatch
        @param1:name: The name the watch was registered under.
        @returns True if a watch of that name existed.
        */
        ScriptUtils::addFunction(vm, unwatch, "unwatch", 2, ".s");
        /**SQFunction
        @name getWatches
        @returns An array of the names currently being watched.
        */
        ScriptUtils::addFunction(vm, getWatches, "getWatches");
        /**SQFunction
        @name isEnabled
        @returns True if the recorder is compiled in, enabled and currently recording.
        */
        ScriptUtils::addFunction(vm, isEnabled, "isEnabled");
        /**SQFunction
        @name start
        @desc Resume recording after a stop().
        */
        ScriptUtils::addFunction(vm, start, "start");
        /**SQFunction
        @name stop
        @desc Pause recording, leaving what has been buffered intact.
        */
        ScriptUtils::addFunction(vm, stop, "stop");
        /**SQFunction
        @name getFramesBuffered
        @returns How many frames the circular buffer currently holds.
        */
        ScriptUtils::addFunction(vm, getFramesBuffered, "getFramesBuffered");
        /**SQFunction
        @name getLastCapturePath
        @returns The directory of the most recent capture, or an empty string if none was taken.
        */
        ScriptUtils::addFunction(vm, getLastCapturePath, "getLastCapturePath");
    }
}

#endif
