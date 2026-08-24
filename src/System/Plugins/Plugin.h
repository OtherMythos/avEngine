#pragma once

#include <string>

namespace AV{

    class Plugin{
    public:
        Plugin(const std::string& pluginName);
        /**
        Virtual so PluginManager::unload, which deletes through a Plugin*, runs
        the derived destructor.
        */
        virtual ~Plugin();

        const std::string& getName() { return mName; };

        virtual void initialise();
        virtual void shutdown();

        /**
        Called exactly once per rendered frame, before the window polls and
        dispatches that frame's input. This is not called per fixed update step,
        so it stays in step with rendering however many logic steps run.

        Use it to refresh anything the engine consults during input dispatch, in
        particular an input layer's capture answer and its liveness counter.
        */
        virtual void frameUpdate();

    private:
        std::string mName;
    };

}
