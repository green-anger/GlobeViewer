#pragma once

#include <memory>

#include "type/TileServer.h"
#include "TileServerBase.h"


namespace gv {


/*!
 * \brief Construct concrete tile servers.
 */
class TileServerFactory
{
public:
    //! Create tile server instance by its identifier.
    static std::unique_ptr<TileServerBase> createTileServer( TileServer );
};


}
