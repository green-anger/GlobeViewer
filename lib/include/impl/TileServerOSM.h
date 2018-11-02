#pragma once

#include <array>
#include <mutex>

#include "TileServerBase.h"


namespace gv {


/*!
 * \brief Implements OpenStreetMap tile server.
 *
 * For documentation for overridden virtual private methods
 * look in the base class TileServerBase.
 */
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

    const std::string name_;                        //!< Server name.
    const std::string domain_;                      //!< Server address.
    const std::string port_;                        //!< Server port.
    const std::array<std::string, 3> subDomain_;    //!< Server subdomains.
    std::mutex mutex_;                              //!< Allows to synchronize mirror cycling.
    unsigned int curSub_;                           //!< Current subdomain.
};


}

