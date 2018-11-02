#pragma once

#include <unordered_map>

#include "type/Tile.h"


namespace gv {

//! Maps tile header to tile body.
using TileMap = std::unordered_map<TileHead, TileBody>;

}


namespace std
{

template <>
struct hash<gv::TileHead>
{
    std::size_t operator()( const gv::TileHead& tile ) const
    {
        using std::size_t;
        using std::hash;

        return ( ( hash<int>()( tile.x ) ^ ( hash<int>()( tile.y ) << 1 ) ) >> 1 ) ^ ( hash<int>()( tile.z ) << 1 );
    }
};

template <>
struct hash<std::pair<int, int>>
{
    std::size_t operator()( const std::pair<int, int>& pair ) const
    {
        using std::size_t;
        using std::hash;

        return ( hash<int>()( pair.first ) ^ ( hash<int>()( pair.second ) << 1 ) );
    }
};

}

