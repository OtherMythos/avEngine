#include "ImguiSlotView.h"

#include "imgui.h"

#include "World/WorldSingleton.h"
#include "World/Slot/SlotManager.h"

namespace AV {
    ImguiSlotView::ImguiSlotView(){
        
    }
    
    void ImguiSlotView::prepareGui(bool *pOpen){
        if(!WorldSingleton::getWorld()) return;
        
        if(!ImGui::Begin("Slot Manager", pOpen)){
            ImGui::End();
            return;
        }
        
        SlotManager::SlotDebugInfo info;
        WorldSingleton::getWorld()->getSlotManager()->getDebugInfo(&info);
        
        ImGui::Text("Current Map Name: %s", WorldSingleton::getCurrentMap().c_str());
        ImGui::Text("Total Chunks: %i", info.totalChunks);
        
        ImGui::Separator();
        if(ImGui::Button("Set origin to player position")){
            WorldSingleton::getWorld()->getSlotManager()->setOrigin(WorldSingleton::getPlayerPosition());
        }
        ImGui::Separator();
        if(ImGui::Button("Set map to Overworld")){
            WorldSingleton::getWorld()->getSlotManager()->setCurrentMap("overworld");
        }
        if(ImGui::Button("Set map to Map")){
            WorldSingleton::getWorld()->getSlotManager()->setCurrentMap("map");
        }
        ImGui::Separator();
        
        ImGui::Text("Max Recipies: %i", SlotManager::mMaxRecipies);
        
        int columns = SlotManager::mMaxRecipies/4;
        //If there is a remainder, there needs to be an extra line added.
        if(SlotManager::mMaxRecipies % 4 != 0) columns++;
        
        SlotManager::SlotRecipeDebugInfo recipeInfo;
        // NB: Future columns API should allow automatic horizontal borders.
        ImGui::Columns(4, NULL, true);
        for (int i = 0; i < 4*columns; i++)
        {
            if (ImGui::GetColumnIndex() == 0)
                ImGui::Separator();
            if(i >= SlotManager::mMaxRecipies){
                ImGui::NextColumn();
                continue;
            }
            WorldSingleton::getWorld()->getSlotManager()->getSlotRecipeDebugInfo(i, &recipeInfo);
            
            ImGui::Text("Index: %i", i);
            ImGui::Text("recipe: %s,%i,%i", recipeInfo.coord.mapName().c_str(), recipeInfo.coord.chunkX(), recipeInfo.coord.chunkY());
            ImGui::Text("Available: %s", recipeInfo.slotAvailable ? "True" : "False");
            ImGui::Text("Ready: %s", recipeInfo.recipeReady ? "True" : "False");
            ImGui::Text("Score: %i", recipeInfo.recipeScore);
            
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::Separator();
        
        
        
        ImGui::End();
    }
}
