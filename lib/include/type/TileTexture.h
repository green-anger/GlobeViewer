#pragma once

#include <tuple>
#include <vector>

#include "type/Tile.h"
#include "type/TileMap.h"


namespace gv {


/*!
 * \brief Holds meta data of map texture.
 *
 * It has everything that need to build a map texture except for
 * image data (bytes), hence it's meta data.
 */
struct TileTexture
{
    std::tuple<int, int> textureSize;   //!< Dimensions (rows, columns) of map texture.
    int tileCount;                      //!< Total number of tiles.
    TileMap tiles;                      //!< Tiles meta data.

    //! Default constructor.
    TileTexture() : textureSize( { 0, 0 } ), tileCount( 0 ), tiles( {} ) {}
   
    //! Default copy constructor.
    TileTexture( const TileTexture& ) = default;

    //! Standard copy assignment operator.
    TileTexture& operator=( const TileTexture& rhs )
    {
        textureSize = rhs.textureSize;
        tileCount = rhs.tileCount;
        tiles = rhs.tiles;
        return *this;
    }
};


}
