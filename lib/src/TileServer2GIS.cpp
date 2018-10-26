#pragma once

#include "TileServer2GIS.h"


namespace gv {


TileServer2GIS::TileServer2GIS()
    : name_( "2GIS" )
    , domain_( ".maps.2gis.com" )
    , port_( "80" )
    , subDomain_( { "tile0", "tile1", "tile2", "tile3", "tile4" } )
    , curSub_( 0 )
{
}


TileServer2GIS::~TileServer2GIS()
{
}


std::string TileServer2GIS::getServerName() const
{
    return name_;
}


std::string TileServer2GIS::getServerPort() const
{
    return port_;
}


std::string TileServer2GIS::getNextMirror()
{
    std::lock_guard<std::mutex> lock( mutex_ );
    curSub_ = ( ++curSub_ ) % std::tuple_size<decltype( subDomain_ )>::value;
    return subDomain_[curSub_] + domain_;
}


std::string TileServer2GIS::getTileTarget( int z, int x, int y ) const
{
    return "/tiles?x=" + std::to_string( x ) + "&y=" + std::to_string( y ) + "&z=" + std::to_string( z );
}


}

