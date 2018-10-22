#pragma once

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/signals2.hpp>

#include "type/TileTexture.h"
#include "type/ViewData.h"


namespace gv {


class Projector;


class MapGenerator
{
public:
    MapGenerator();
    ~MapGenerator();

    void init( ViewData );
    void updateGlobe();
    void updateViewData( ViewData );
    void getTiles( const std::vector<TileImage>& );

    boost::signals2::signal<std::shared_ptr<Projector>()> getProjector;
    boost::signals2::signal<void( std::vector<TileHead> )> requestTiles;
    boost::signals2::signal<void( bool )> mapReady;
    boost::signals2::signal<void( std::vector<GLfloat> /*vbo*/, int /*texW*/, int /*texH*/,
        std::vector<unsigned char> /*data*/)> updateMapTexture;

private:
    void checkStates();
    void cleanupCheck();
    void regenerateMap();
    int lonToTileX( double lon, int z ) const;
    int latToTileY( double lat, int z ) const;
    double tileXToLon( int x, int z ) const;
    double tileYToLat( int y, int z ) const;
    bool visiblePoint( double& lon, double& lat );
    std::vector<TileHead> findTilesToProcess( int z, int x, int y );
    bool tileVisible( TileHead );
    void composeTileTexture( const std::vector<TileHead>& );
    void vboFromTileTexture( TileTexture );
    void finalize();

    boost::asio::io_context ioc_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_;
    std::vector<std::thread> threads_;

    std::shared_ptr<Projector> projector_;
    ViewData viewData_;
    ViewData newViewData_;
    TileTexture tileTex_;

    std::vector<GLfloat> vbo_;
    std::vector<unsigned char> data_;

    std::mutex mutexState_;
    bool active_;
    bool pending_;
    std::atomic<bool> gotTiles_;
    std::atomic<bool> calcedVbo_;
};


}
