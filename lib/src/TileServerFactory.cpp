#pragma once

#include <stdexcept>

#include "TileServer2GIS.h"
#include "TileServerFactory.h"
#include "TileServerOSM.h"


namespace gv {


std::unique_ptr<TileServerBase> TileServerFactory::createTileServer( TileServer ts )
{
    switch ( ts )
    {
    case TileServer::OSM: return std::make_unique<TileServerOSM>();
    case TileServer::GIS: return std::make_unique<TileServer2GIS>();
    }

    throw std::logic_error( "Unknown TileServer" );
}


}
