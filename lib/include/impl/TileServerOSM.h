#pragma once

#include <array>
#include <mutex>

#include "TileServerBase.h"


namespace gv {


class TileServerOSM : public TileServerBase
{
public:
    TileServerOSM();
    ~TileServerOSM();

private:
    virtual std::string getServerName() const override;
    virtual std::string getServerPort() const override;
    virtual std::string getNextMirror() override;
    virtual std::string getTileTarget( int z, int x, int y ) const override;

    const std::string name_;
    const std::string domain_;
    const std::string port_;
    const std::array<std::string, 3> subDomain_;
    std::mutex mutex_;
    unsigned int curSub_;
};


}

