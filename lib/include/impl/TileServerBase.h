#pragma once

#include <string>


namespace gv {


/*!
 * \brief Abstract class enforcing Template Method.
 *
 * Class has non-virtual public API that calls virtual private
 * methods in any desirable order. Implementing classes can
 * only override private methods. So TileServerBase defines
 * logic and implementers defines small steps and provide data.
 */
class TileServerBase
{
public:
    virtual ~TileServerBase();

    //! Provide tile server name.
    std::string serverName() const;

    //! Provide tile server port.
    std::string serverPort() const;

    //! Provide a mirror of the tile server.
    std::string nextMirror();

    //! Provide tile image address on the tile server.
    std::string tileTarget( int z, int x, int y ) const;

private:
    //! Implements serverName.
    virtual std::string getServerName() const = 0;

    //! Implements serverPort.
    virtual std::string getServerPort() const = 0;

    //! \brief Implements nextMirror.
    //! \warning It must be thread safe.
    virtual std::string getNextMirror() = 0;

    //! Implements tileTarget.
    virtual std::string getTileTarget( int z, int x, int y ) const = 0;
};


}
