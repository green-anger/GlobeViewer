#pragma once

#include <array>
#include <mutex>

#include "TileServerBase.h"


namespace gv {


class TileServer2GIS : public TileServerBase
{
public:
    TileServer2GIS();
    ~TileServer2GIS();

private:
    virtual std::string getServerName() const override;
    virtual std::string getServerPort() const override;
    virtual std::string getNextMirror() override;
    virtual std::string getTileTarget( int z, int x, int y ) const override;

    const std::string name_;
    const std::string domain_;
    const std::string port_;
    const std::array<std::string, 5> subDomain_;
    std::mutex mutex_;
    unsigned int curSub_;
};


}

