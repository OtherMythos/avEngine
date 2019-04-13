#include "ImguiEntityView.h"

#include "imgui.h"
#include "Logger/Log.h"

#include "World/WorldSingleton.h"
#include "World/Entity/EntityManager.h"

namespace AV {
    ImguiEntityView::ImguiEntityView(){
        
    }
    
    void ImguiEntityView::prepareGui(bool* pOpen){
        
        if(!ImGui::Begin("Entity Manager", pOpen)){
            ImGui::End();
            return;
        }
        
        EntityManager::EntityDebugInfo info;
        WorldSingleton::getWorld()->getEntityManager()->getDebugInfo(&info);
        
        ImGui::Text("Total Entities: %i", info.totalEntities);
        
        ImGui::End();
    }
}
