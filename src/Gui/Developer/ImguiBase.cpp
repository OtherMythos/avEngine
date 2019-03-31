#include "ImguiBase.h"

#include <iostream>
#include "Gui/Developer/imguiOgre/ImguiManager.h"
#include "Input/Input.h"
#include <OgreRoot.h>

#include "World/WorldSingleton.h"

#include "Logger/Log.h"

namespace AV{
    ImguiBase::ImguiBase(){

    }

    void ImguiBase::initialise(Ogre::SceneManager *sceneManager){
        //ImGuiIO& io = ImGui::GetIO();

        ImguiManager::getSingleton().init(sceneManager);

        _setupImguiStyle();

        Ogre::Root::getSingleton().addFrameListener(this);
    }

    void ImguiBase::_setupImguiStyle(){
        ImGuiStyle * style = &ImGui::GetStyle();
        style->Alpha = 0.8;
        style->WindowRounding = 0;
        style->ScaleAllSizes(2);
    }

    bool ImguiBase::frameRenderingQueued(const Ogre::FrameEvent& evt){
        ImguiManager* manager = ImguiManager::getSingletonPtr();
        manager->render();

        _processInput();
        manager->newFrame(1000/60);


        bool show_demo_window;
    	ImGui::ShowDemoWindow(&show_demo_window);

        _showOverlay();


    	return true;
    }

    void ImguiBase::_processInput(){
        ImGuiIO& io = ImGui::GetIO();

        io.MousePos = ImVec2((float)Input::getMouseX(), (float)Input::getMouseY());
        io.MouseDown[0] = Input::getMouseButton(0);
        io.MouseDown[1] = Input::getMouseButton(1);

        int mouseVal = 0;
        if(Input::getMouseWheel() < 0) mouseVal = -1;
        if(Input::getMouseWheel() > 0) mouseVal = 1;

        io.MouseWheel += mouseVal;
    }

    void ImguiBase::_showOverlay(){
        ImGui::SetNextWindowBgAlpha(0.3f);
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(10022, 680), ImGuiCond_FirstUseEver);
        bool pOpen;
        ImGui::Begin("Overlay", &pOpen,ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav); // begin window

        const SlotPosition& origin = WorldSingleton::getOrigin();
        const SlotPosition& playerPos = WorldSingleton::getPlayerPosition();
        Ogre::Vector3 ogrePos = playerPos.toOgre();
        ImGui::Text("Origin: %i %i (%f %f %f)", origin.chunkX(), origin.chunkY(), (float)origin.position().x, (float)origin.position().y, (float)origin.position().z);
        ImGui::Text("Player Slot Position: %i %i (%f %f %f)", playerPos.chunkX(), playerPos.chunkY(), (float)playerPos.position().x, (float)playerPos.position().y, (float)playerPos.position().z);
        ImGui::Text("Player ogre Position: %f %f %f", (float)ogrePos.x, (float)ogrePos.y, (float)ogrePos.z);

        ImGui::Separator();

        if (ImGui::CollapsingHeader("Views")){
            static bool slotManagerCheck = true;
            ImGui::Checkbox("SlotManager", &slotManagerCheck);

            static bool entityManagerCheck = true;
            ImGui::Checkbox("Entities", &entityManagerCheck);
        }

        ImGui::Separator();

        ImGui::End(); // end window
    }
}
