#include "gtest/gtest.h"

#include "System/Plugins/Plugin.h"
#include "System/Plugins/PluginManager.h"

#include <string>
#include <vector>

namespace {
    class RecordingPlugin final : public AV::Plugin {
    public:
        explicit RecordingPlugin(std::vector<std::string>* calls)
            : AV::Plugin("recording"), mCalls(calls) {}

        ~RecordingPlugin() override {
            mCalls->push_back("destroy");
        }

        void initialise() override {
            mCalls->push_back("initialise");
        }

        void earlyShutdown() override {
            mCalls->push_back("earlyShutdown");
        }

        void shutdown() override {
            mCalls->push_back("shutdown");
        }

    private:
        std::vector<std::string>* mCalls;
    };
}

TEST(PluginManagerTests, runsBothShutdownPhasesBeforeDestroyingPlugins){
    std::vector<std::string> calls;

    AV::PluginManager::registerPlugin(new RecordingPlugin(&calls));
    AV::PluginManager::earlyShutdown();
    AV::PluginManager::shutdown();
    AV::PluginManager::unload();

    const std::vector<std::string> expected{
        "initialise", "earlyShutdown", "shutdown", "destroy"
    };
    EXPECT_EQ(expected, calls);
}
