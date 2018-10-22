#pragma once

#include <tuple>
#include <vector>

#include "type/Tile.h"
#include "type/TileMap.h"


namespace gv {


struct TileTexture
{
    std::tuple<int, int> textureSize;   //!< (rows, cols) of tile texture in tile numbers
    int tileCount;                      //!< total number of tiles
    int tileFilled;                     //!< number of tiles with TileData filled (got its image)
    TileMap tiles;                      //!< tiles themselves

    TileTexture() : textureSize( { 0, 0 } ), tileCount( 0 ), tileFilled( 0 ), tiles( {} ) {}
    TileTexture( const TileTexture& ) = default;

    TileTexture& operator=( const TileTexture& rhs )
    {
        textureSize = rhs.textureSize;
        tileCount = rhs.tileCount;
        tileFilled = rhs.tileFilled;
        tiles = rhs.tiles;
        return *this;
    }
};


}
