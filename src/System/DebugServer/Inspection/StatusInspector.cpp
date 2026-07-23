#ifdef DEBUG_SERVER

#include "StatusInspector.h"

#include "System/BaseSingleton.h"
#include "System/SystemSetup/SystemSettings.h"
#include "System/EnginePrerequisites.h"
#include "Window/Window.h"
#include "git_version.h"

#include <string>

namespace AV{
    static const char* renderSystemName(SystemSettings::RenderSystemTypes type){
        switch(type){
            case SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL: return "OpenGL";
            case SystemSettings::RenderSystemTypes::RENDER_SYSTEM_METAL: return "Metal";
            case SystemSettings::RenderSystemTypes::RENDER_SYSTEM_D3D11: return "D3D11";
            case SystemSettings::RenderSystemTypes::RENDER_SYSTEM_VULKAN: return "Vulkan";
            default: return "Unset";
        }
    }

    void StatusInspector::writeStatus(rapidjson::Document& doc, double uptimeSeconds){
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();

        const std::string engineVersion =
            std::to_string(ENGINE_VERSION_MAJOR) + "." +
            std::to_string(ENGINE_VERSION_MINOR) + "." +
            std::to_string(ENGINE_VERSION_PATCH) + " " + ENGINE_VERSION_SUFFIX;
        doc.AddMember("engineVersion", rapidjson::Value(engineVersion.c_str(), allocator), allocator);
        doc.AddMember("gitHash", rapidjson::Value(kGitHash, allocator), allocator);

        //Must track the version reported by the /api catalog in DebugServer.cpp.
        doc.AddMember("apiVersion", 2, allocator);
        doc.AddMember("uptimeSeconds", uptimeSeconds, allocator);

        const std::string& project = SystemSettings::getProjectName();
        doc.AddMember("project", rapidjson::Value(project.c_str(), allocator), allocator);

        doc.AddMember("renderSystem",
            rapidjson::Value(renderSystemName(SystemSettings::getCurrentRenderSystem()), allocator), allocator);

        const PerformanceStats& stats = BaseSingleton::getPerformanceStats();
        doc.AddMember("fps", stats.fps, allocator);
        doc.AddMember("avgFps", stats.avgFPS, allocator);
        doc.AddMember("frameTimeMs", stats.frameTime, allocator);

        rapidjson::Value window(rapidjson::kObjectType);
        Window* win = BaseSingleton::getWindow();
        if(win){
            window.AddMember("width", win->getWidth(), allocator);
            window.AddMember("height", win->getHeight(), allocator);
            window.AddMember("title", rapidjson::Value(win->getTitle().c_str(), allocator), allocator);
        }
        doc.AddMember("window", window, allocator);
    }
}

#endif
