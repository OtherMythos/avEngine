#pragma once

namespace AV{
    class PatternHelper{
    public:
        PatternHelper() = delete;
        ~PatternHelper() = delete;

        static void GenPerlinNoise(int width, int height, float* outNoise, float frequency=0.1, int depth=4);

        static void setPatternSeed(int seed);

    private:
        static int mSeed;

    public:
        static int getSeed() { return mSeed; }
    };
}
