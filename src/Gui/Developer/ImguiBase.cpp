#include "ImguiBase.h"

#include <iostream>
#include "Gui/Developer/imguiOgre/ImguiManager.h"
#include "Input/Input.h"
#include <OgreRoot.h>

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
        style->ScaleAllSizes(2);
    }

    bool ImguiBase::frameRenderingQueued(const Ogre::FrameEvent& evt){
        ImguiManager* manager = ImguiManager::getSingletonPtr();
        manager->render();

        processInput();
        manager->newFrame(1000/60);


        bool show_demo_window;
    	ImGui::ShowDemoWindow(&show_demo_window);

    	return true;
    }

    void ImguiBase::processInput(){
        ImGuiIO& io = ImGui::GetIO();

        io.MousePos = ImVec2((float)Input::getMouseX(), (float)Input::getMouseY());
        io.MouseDown[0] = Input::getMouseButton(0);
        io.MouseDown[1] = Input::getMouseButton(1);

        int mouseVal = 0;
        if(Input::getMouseWheel() < 0) mouseVal = -1;
        if(Input::getMouseWheel() > 0) mouseVal = 1;

        io.MouseWheel += mouseVal;
    }
}
