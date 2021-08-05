#pragma once

#include <string>

namespace AV{
    struct MapNavMetaParserData{
        std::string meshName;
        float tileSize;
        float cellSize;

        float calculatedTileSize;
        float calculatedTilesInChunk;
    };
}
