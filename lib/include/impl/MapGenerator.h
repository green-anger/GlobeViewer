#pragma once

#include <vector>

#include <boost/signals2.hpp>

#include "type/TileTexture.h"


namespace gv {


class MapGenerator
{
public:
    MapGenerator();
    ~MapGenerator();

    void regenerateMap();

    boost::signals2::signal<float()> getUnitInMeter;
    boost::signals2::signal<float()> getMeterInPixel;
    boost::signals2::signal<int()> getMapZoomLevel;
    boost::signals2::signal<std::tuple<double, double, double, double>()> getViewBorder;
    boost::signals2::signal<std::tuple<int, int>()> getViewPixelSize;
    boost::signals2::signal<std::tuple<double, double>()> getProjectionCenter;
    boost::signals2::signal<std::tuple<bool, double, double>( double, double )> getInvProjection;
    boost::signals2::signal<std::tuple<bool, double, double>( double, double )> getFwdProjection;
    boost::signals2::signal<void( TileTexture )> sendTileTexture;
    boost::signals2::signal<void( std::vector<TileHead> )> requestTiles;

private:
    int lonToTileX( double lon, int z ) const;
    int latToTileY( double lat, int z ) const;
    double tileXToLon( int x, int z ) const;
    double tileYToLat( int y, int z ) const;
    bool visiblePoint( double& lon, double& lat );
    std::vector<TileHead> findTilesToProcess( int z, int x, int y );
    bool tileVisible( TileHead );
    void composeTileTexture( const std::vector<TileHead>& );
};


}
