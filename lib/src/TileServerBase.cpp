#include "TileServerBase.h"


namespace gv {


TileServerBase::~TileServerBase()
{
}


std::string TileServerBase::serverName() const
{
    return getServerName();
}


std::string TileServerBase::serverPort() const
{
    return getServerPort();
}


std::string TileServerBase::nextMirror()
{
    return getNextMirror();
}


std::string TileServerBase::tileTarget( int z, int x, int y ) const
{
    return getTileTarget( z, x, y );
}


}
