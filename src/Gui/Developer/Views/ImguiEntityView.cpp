#include "ImguiEntityView.h"

#include "imgui.h"
#include "Logger/Log.h"

#include "World/WorldSingleton.h"
#include "World/Entity/EntityManager.h"
#include "World/Entity/Tracker/EntityTracker.h"
#include "World/Entity/Tracker/EntityTrackerChunk.h"
#include "System/SystemSetup/SystemSettings.h"

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
        ImGui::Text("Tracked Entities: %i", info.trackedEntities);
        
        _drawChunkCanvas();
        
        ImGui::End();
    }
    
    void ImguiEntityView::_drawChunkCanvas(){
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        
        //Won't work with origin switching but good enough for now.
        Ogre::Vector3 playerPos = WorldSingleton::getPlayerPosition().toOgre();
        int movX = -playerPos.x;
        int movY = -playerPos.z;
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_size(200, 200);
        ImVec2 canvasEnd_pos(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y);
        draw_list->AddRectFilled(ImVec2(canvas_pos.x, canvas_pos.y), ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(255, 255, 255, 255));
        
        //Depending on the size of the radius it needs to be put into proportion.
        //So the circle will always take up 50% of the box.
        //If the circle is 200 units, in radius, the boxes need to take that into account.
        
        float circleRad = WorldSingleton::getPlayerLoadRadius();
        float slotSize = SystemSettings::getWorldSlotSize();
        
        
        float worldCoord = canvas_size.x / (circleRad * 4);
        
        const int incr = slotSize * worldCoord;
        for(int x = 0; x < canvas_size.x / incr; x++){
            draw_list->AddLine(ImVec2(canvas_pos.x + (movX % incr) + x * incr, canvas_pos.y), ImVec2(canvas_pos.x + (movX % incr) + x * incr, canvasEnd_pos.y), IM_COL32(0, 0, 0, 255));
        }
        for(int y = 0; y < canvas_size.y / incr; y++){
            draw_list->AddLine(ImVec2(canvas_pos.x, canvas_pos.y + (movY % incr) + y * incr), ImVec2(canvasEnd_pos.x, canvas_pos.y + (movY % incr) + y * incr), IM_COL32(0, 0, 0, 255));
        }
        
        
        //Draw rectangles for the highlighted squares.
        const std::map<EntityTracker::ChunkEntry, EntityTrackerChunk*>& container = WorldSingleton::getWorld()->getEntityManager()->getEntityTracker()->mEChunks;
        auto it = container.begin();
        while(it != container.end()){
            int squareX = (*it).first.first;
            int squareY = (*it).first.second;
            ImVec2 startPos = ImVec2(canvas_pos.x + (squareX * incr) + movX, canvas_pos.y + (squareY * incr) + movY);
            ImVec2 endPos = ImVec2(canvas_pos.x + incr + (squareX * incr) + movX, canvas_pos.y + incr + (squareY * incr) + movY);
            
            //Add half of the canvas as offset to the cubes, because the radius is drawn at the centre of the canvas.
            startPos.x += canvas_size.x / 2;
            startPos.y += canvas_size.y / 2;
            endPos.x += canvas_size.x / 2;
            endPos.y += canvas_size.y / 2;
            
            //Check if the rect should be drawn
            if(startPos.x < canvas_pos.x - incr ||
               startPos.y < canvas_pos.y - incr ||
               endPos.x > canvasEnd_pos.x + incr ||
               endPos.y > canvasEnd_pos.y + incr) {
                it++;
                continue;
            }
            //Check if the rect needs to have its size changed so it fits in the canvas
            if(startPos.x < canvas_pos.x) startPos.x = canvas_pos.x;
            if(startPos.y < canvas_pos.y) startPos.y = canvas_pos.y;
            if(endPos.x > canvasEnd_pos.x) endPos.x = canvasEnd_pos.x;
            if(endPos.y > canvasEnd_pos.y) endPos.y = canvasEnd_pos.y;
            
            draw_list->AddRectFilled(startPos, endPos, IM_COL32(100, 100, 100, 255));
            
            std::string s = std::to_string((*it).second->getEntityCount());
            
            draw_list->AddText(startPos, IM_COL32(0, 0, 0, 255), s.c_str());
            
            it++;
        }
        
        draw_list->AddCircle(ImVec2(canvas_pos.x + canvas_size.x / 2, canvas_pos.y + canvas_size.y / 2), 50, IM_COL32(0, 0, 0, 255), 32, 1);
    }
}
