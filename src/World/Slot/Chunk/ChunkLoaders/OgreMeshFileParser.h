#pragma once

#include <vector>

#include "OgreMeshRecipeData.h"

namespace AV{
  class OgreMeshFileParser{
  public:
    OgreMeshFileParser();
    ~OgreMeshFileParser();

    void parseFile(std::vector<OgreMeshRecipeData>* store);

  };
}
