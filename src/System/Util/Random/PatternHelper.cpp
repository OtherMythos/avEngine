#include "PatternHelper.h"

#include "PerlinNoise.h"

namespace AV{

    int PatternHelper::mSeed = 0;

    void PatternHelper::setPatternSeed(int seed){
        mSeed = seed;
    }

    void PatternHelper::GenPerlinNoise(int width, int height, float* outNoise){
        for(int y = 0; y < width; y++){
            for(int x = 0; x < height; x++){
                *outNoise++ = Perlin::perlin2d(x, y, 0.1, 4);
            }
        }
    }

}