#pragma once

#include <cmath>
#include <algorithm>
#include <set>
#include "System/EnginePrerequisites.h"

namespace AV{

    typedef unsigned int CollisionEntryId;
    typedef unsigned long long CollisionPackedResult;

    static const CollisionEntryId COLLISION_ENTRY_ID_INVALID = 0xFFFFFFFF;
    static const CollisionPackedResult COLLISION_PACKED_RESULT_INVALID = 0xFFFFFFFFFFFFFFFF;

    enum class CollisionEntryType : uint32{
        either,
        sender,
        receiver
    };

    inline bool checkCircleCollision(float x1, float y1, float rad1, float x2, float y2, float rad2){
        float dx = x1 - x2;
        float dy = y1 - y2;
        float radiusSum = rad1 + rad2;

        return (dx * dx + dy * dy) <= (radiusSum * radiusSum);
    }

    inline bool checkCircleRectangleCollision(float cx, float cy, float radius, float rx, float ry, float rw_half, float rh_half){
        float closestX = std::max(rx - rw_half, std::min(cx, rx + rw_half));
        float closestY = std::max(ry - rh_half, std::min(cy, ry + rh_half));

        float dx = cx - closestX;
        float dy = cy - closestY;

        return (dx * dx + dy * dy) <= (radius * radius);
    }

    inline bool checkRectangleCollision(float x1, float y1, float w1_half, float h1_half, float x2, float y2, float w2_half, float h2_half){
        return !(x1 + w1_half < x2 - w2_half || x2 + w2_half < x1 - w1_half ||
                 y1 + h1_half < y2 - h2_half || y2 + h2_half < y1 - h1_half);
    }

    //Rotated rectangle collision detection using Separating Axis Theorem (SAT)
    //Note: w_half and h_half are half-width and half-height respectively
    inline bool checkRotatedRectangleCollision(float x1, float y1, float w1_half, float h1_half, float rot1, float x2, float y2, float w2_half, float h2_half, float rot2){
        //Convert angles to radians if necessary (assuming input is already in radians)
        float cos1 = std::cos(rot1);
        float sin1 = std::sin(rot1);
        float cos2 = std::cos(rot2);
        float sin2 = std::sin(rot2);

        //Get corner points for rectangle 1
        float corners1[4][2] = {
            {-w1_half, -h1_half}, {w1_half, -h1_half}, {w1_half, h1_half}, {-w1_half, h1_half}
        };

        //Get corner points for rectangle 2
        float corners2[4][2] = {
            {-w2_half, -h2_half}, {w2_half, -h2_half}, {w2_half, h2_half}, {-w2_half, h2_half}
        };

        //Rotate and translate corners for rect1
        float rotated1[4][2];
        for(int i = 0; i < 4; i++){
            rotated1[i][0] = corners1[i][0] * cos1 - corners1[i][1] * sin1 + x1;
            rotated1[i][1] = corners1[i][0] * sin1 + corners1[i][1] * cos1 + y1;
        }

        //Rotate and translate corners for rect2
        float rotated2[4][2];
        for(int i = 0; i < 4; i++){
            rotated2[i][0] = corners2[i][0] * cos2 - corners2[i][1] * sin2 + x2;
            rotated2[i][1] = corners2[i][0] * sin2 + corners2[i][1] * cos2 + y2;
        }

        //Check axes from rectangle 1
        for(int i = 0; i < 2; i++){
            float axisX = -(rotated1[(i+1)%4][1] - rotated1[i][1]);
            float axisY = rotated1[(i+1)%4][0] - rotated1[i][0];
            float len = std::sqrt(axisX * axisX + axisY * axisY);
            if(len < 1e-6f) continue;
            axisX /= len;
            axisY /= len;

            float min1 = 1e9f, max1 = -1e9f;
            float min2 = 1e9f, max2 = -1e9f;

            for(int j = 0; j < 4; j++){
                float proj = rotated1[j][0] * axisX + rotated1[j][1] * axisY;
                min1 = std::min(min1, proj);
                max1 = std::max(max1, proj);
            }

            for(int j = 0; j < 4; j++){
                float proj = rotated2[j][0] * axisX + rotated2[j][1] * axisY;
                min2 = std::min(min2, proj);
                max2 = std::max(max2, proj);
            }

            if(max1 < min2 - 1e-6f || max2 < min1 - 1e-6f) return false;
        }

        //Check axes from rectangle 2
        for(int i = 0; i < 2; i++){
            float axisX = -(rotated2[(i+1)%4][1] - rotated2[i][1]);
            float axisY = rotated2[(i+1)%4][0] - rotated2[i][0];
            float len = std::sqrt(axisX * axisX + axisY * axisY);
            if(len < 1e-6f) continue;
            axisX /= len;
            axisY /= len;

            float min1 = 1e9f, max1 = -1e9f;
            float min2 = 1e9f, max2 = -1e9f;

            for(int j = 0; j < 4; j++){
                float proj = rotated1[j][0] * axisX + rotated1[j][1] * axisY;
                min1 = std::min(min1, proj);
                max1 = std::max(max1, proj);
            }

            for(int j = 0; j < 4; j++){
                float proj = rotated2[j][0] * axisX + rotated2[j][1] * axisY;
                min2 = std::min(min2, proj);
                max2 = std::max(max2, proj);
            }

            if(max1 < min2 - 1e-6f || max2 < min1 - 1e-6f) return false;
        }

        return true;
    }

    inline bool checkCircleRotatedRectangleCollision(float cx, float cy, float radius, float rx, float ry, float rw_half, float rh_half, float rot){
        float cos_rot = std::cos(rot);
        float sin_rot = std::sin(rot);

        //Transform circle centre to rectangle's local space
        float dx = cx - rx;
        float dy = cy - ry;
        float localX = dx * cos_rot + dy * sin_rot;
        float localY = -dx * sin_rot + dy * cos_rot;

        //Find closest point on rectangle in local space
        float closestX = std::max(-rw_half, std::min(localX, rw_half));
        float closestY = std::max(-rh_half, std::min(localY, rh_half));

        //Distance from circle centre to closest point
        float distX = localX - closestX;
        float distY = localY - closestY;

        return (distX * distX + distY * distY) <= (radius * radius);
    }

}
