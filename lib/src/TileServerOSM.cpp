#pragma once

#include "TileServerOSM.h"


namespace gv {


TileServerOSM::TileServerOSM()
    : name_( "OpenStreetMap" )
    , domain_( ".tile.openstreetmap.org" )
    , port_( "80" )
    , subDomain_( { "a", "b", "c" } )
    , curSub_( 0 )
{
}


TileServerOSM::~TileServerOSM()
{
}


std::string TileServerOSM::getServerName() const
{
    return name_;
}


std::string TileServerOSM::getServerPort() const
{
    return port_;
}


std::string TileServerOSM::getNextMirror()
{
    std::lock_guard<std::mutex> lock( mutex_ );
    curSub_ = ( ++curSub_ ) % std::tuple_size<decltype( subDomain_ )>::value;
    return subDomain_[curSub_] + domain_;
}


std::string TileServerOSM::getTileTarget( int z, int x, int y ) const
{
    return "/" + std::to_string( z ) + "/" + std::to_string( x ) + "/" + std::to_string( y ) + ".png";
}


}

