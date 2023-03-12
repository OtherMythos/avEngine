#include "PatternHelper.h"

#include "PerlinNoise.h"

namespace AV{

    int PatternHelper::mSeed = 0;

    void PatternHelper::setPatternSeed(int seed){
        mSeed = seed;
    }

    void PatternHelper::GenPerlinNoise(int width, int height, float* outNoise, float frequency, int depth){
        for(int y = 0; y < width; y++){
            for(int x = 0; x < height; x++){
                *outNoise++ = Perlin::perlin2d(x, y, frequency, depth);
            }
        }
    }

}
