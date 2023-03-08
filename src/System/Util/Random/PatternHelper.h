#pragma once

namespace AV{
    class PatternHelper{
    public:
        PatternHelper() = delete;
        ~PatternHelper() = delete;

        static void GenPerlinNoise(int width, int height, float* outNoise);
    };
}
