#include "TileServerBase.h"


namespace gv {


TileServerBase::~TileServerBase()
{
}


/*!
* \return Tile server address.
*/
std::string TileServerBase::serverName() const
{
    return getServerName();
}


/*!
* \return Tile server port.
*/
std::string TileServerBase::serverPort() const
{
    return getServerPort();
}


/*!
 * If the tile server has several mirrors it's best to cycle
 * through them every call so that the caller will have
 * mirrors uniformly divided.
 *
 * \return Mirror address.
 */
std::string TileServerBase::nextMirror()
{
    return getNextMirror();
}


/*!
* \return Tile image address.
*/
std::string TileServerBase::tileTarget( int z, int x, int y ) const
{
    return getTileTarget( z, x, y );
}


}
