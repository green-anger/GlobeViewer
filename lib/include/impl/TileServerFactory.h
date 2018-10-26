#pragma once

#include <memory>

#include "type/TileServer.h"
#include "TileServerBase.h"


namespace gv {


class TileServerFactory
{
public:
    static std::unique_ptr<TileServerBase> createTileServer( TileServer );
};


}
