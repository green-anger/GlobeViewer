#pragma once

#include <tuple>
#include <vector>

#include "type/Tile.h"


namespace gv {


struct TileTexture
{
    std::tuple<int, int> textureSize;   //!< (width, height) of tile texture in tile numbers
    int tileCount;                      //!< total number of tiles
    int tileFilled;                     //!< number of tiles with TileData filled (got its image)
    std::vector<Tile> tiles;            //!< tiles themselves
};


}
