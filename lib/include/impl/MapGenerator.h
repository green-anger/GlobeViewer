#pragma once

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/signals2.hpp>

#include "type/TileServer.h"
#include "type/TileTexture.h"
#include "type/ViewData.h"


namespace gv {


class Projector;


/*!
 * \brief Generates a single texture from several map tiles.
 *
 * MapGenerator gets signal that something is changed and map texture
 * needs to be regenerated. First of all it determines which map tiles
 * are currently visible at the view and requests them from the system.
 * Once they arrive MapGenerator generates new texture in memory
 * (which is a simple vector of chars) and fills it with new tiles.
 * Then it send texture in memory  to the system.
 *
 * MapGenerator is one of few classes that do a lot of work in a separate
 * thread. That allows for non-blocking behaviour but also leads to
 * delays in displaying the map.
 */
class MapGenerator
{
public:
    MapGenerator();
    ~MapGenerator();

    //! Initialize MapGenerator.
    void init( ViewData );

    //! Notification of rotating the Globe.
    void updateGlobe();

    //! Notification of updating viewport.
    void updateViewData( ViewData );

    //! Notification of changing tile server.
    void updateTileServer( TileServer );

    //! Receiving new map tiles.
    void getTiles( const std::vector<TileImage>& );

    //! Request for pointer to Projector instance.
    boost::signals2::signal<std::shared_ptr<Projector>()> getProjector;

    //! Request for map tiles from particular tile server.
    boost::signals2::signal<void( std::vector<TileHead>, TileServer )> requestTiles;
    
    //! Signal that map is not ready for rendering.
    boost::signals2::signal<void()> mapNotReady;

    //! Send new texture data.
    boost::signals2::signal<void( std::vector<GLfloat> /*vbo*/, int /*texW*/, int /*texH*/,
        std::vector<unsigned char> /*data*/)> updateMapTexture;

private:
    //! Check and modify active_ and pending_ states before generating new map texture.
    void checkStates();

    //! Check and modify active_ and pending_ states after generating new map texture.
    void cleanupCheck();

    //! Generate new map texture.
    void regenerateMap();

    //! Convert longitude to tile coordinate x.
    int lonToTileX( double lon, int z ) const;

    //! Convert latitude to tile coordinate y.
    int latToTileY( double lat, int z ) const;

    //! Convert tile coordinate x to longitude.
    double tileXToLon( int x, int z ) const;

    //! Convert tile coordinate y to latitude.
    double tileYToLat( int y, int z ) const;

    //! Check if a point is visible in current projection.
    bool visiblePoint( double& lon, double& lat );

    //! Find all map tiles that need to be requested for further processing.
    std::vector<TileHead> findTilesToProcess( int z, int x, int y );

    //! Check if a tile is visible in current projection.
    bool tileVisible( TileHead );

    //! Build meta data of a new map texture based on tile headers.
    void composeTileTexture( const std::vector<TileHead>& );

    //! Create vertex buffer object for a new map texture.
    void vboFromTileTexture( TileTexture );

    //! Check if everything is ready for a new map texture.
    void finalize();

    boost::asio::io_context ioc_;           //!< Allows implementing task queue.
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_; //!< Provides work to ioc and doesn't let it stop.
    std::vector<std::thread> threads_;      //!< Vector of worker thread.

    std::shared_ptr<Projector> projector_;  //!< Pointer to Projector instance.
    ViewData viewData_;                     //!< Current viewport dimensions data.
    ViewData newViewData_;                  //!< Newly arrived viewport dimensions data.
    TileServer tileServerType_;             //!< Current server of map tiles.
    TileServer newTileServerType_;          //!< Newly arrived server of map tiles.
    TileTexture tileTex_;                   //!< Meta data of texture currently being generated.

    std::vector<GLfloat> vbo_;              //!< Vertex buffer object for new texture.
    std::vector<unsigned char> data_;       //!< New texture data (plain bytes).

    std::mutex mutexState_;                 //!< For state synchronization.
    bool active_;                           //!< Indicator of new texture being generated.
    bool pending_;                          //!< Indicator of existence of new texture generate request.
    std::atomic<bool> gotTiles_;            //!< Indicator of readiness of map tiles for currently generating texture.
    std::atomic<bool> calcedVbo_;           //!< Indicator of readiness of vertex buffer object for currently generating texture.
};


}
