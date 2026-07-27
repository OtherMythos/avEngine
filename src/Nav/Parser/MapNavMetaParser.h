#pragma once

#include <vector>
#include "MapNavMetaParserData.h"

class dtNavMesh;

namespace AV{

    /**
    A utility class to parse nav mesh metadata files.
    A file is provided for each map describing what sort of nav meshes it contains.
    */
    class MapNavMetaParser{
    public:
        MapNavMetaParser();

        /**
        Parse a meta file, populating the output array with its data.
        @returns True or false depending on whether the file was parsed succesfully.
        */
        bool parseFile(const std::string& filePath, std::vector<MapNavMetaParserData>& out);
    };
}
