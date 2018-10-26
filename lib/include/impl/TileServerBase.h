#pragma once

#include <string>


namespace gv {


class TileServerBase
{
public:
    virtual ~TileServerBase();

    std::string serverName() const;
    std::string serverPort() const;
    std::string nextMirror();
    std::string tileTarget( int z, int x, int y ) const;

private:
    virtual std::string getServerName() const = 0;
    virtual std::string getServerPort() const = 0;
    virtual std::string getNextMirror() = 0;    //<- MUST be thread safe
    virtual std::string getTileTarget( int z, int x, int y ) const = 0;
};


}
