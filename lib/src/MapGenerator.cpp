#include <algorithm>

#include "Defines.h"
#include "MapGenerator.h"
#include "ThreadSafePrinter.hpp"


using TSP = alt::ThreadSafePrinter<alt::MarkPolicy>;
using namespace defs;


namespace gv {


MapGenerator::MapGenerator()
{
}


MapGenerator::~MapGenerator()
{
}


void MapGenerator::regenerateMap()
{
    double lon;
    double lat;

    if ( !visiblePoint( lon, lat ) )
    {
        TSP() << "No visible point!";
        return;
    }

    TSP() << "Visible point at: " << lon << ":" << lat;

    int mapZoomLevel = *getMapZoomLevel();
    int x = lonToTileX( lon, mapZoomLevel );
    int y = latToTileY( lat, mapZoomLevel );

    TSP() << "zoom, tile: " << mapZoomLevel << ", " << x << ":" << y;

    TSP() << "Testing for level 0\n"
        << "0:0 " << tileXToLon( 0, 0 ) << ":" << tileYToLat( 0, 0 ) << "\n"
        << "1:0 " << tileXToLon( 1, 0 ) << ":" << tileYToLat( 0, 0 ) << "\n"
        << "0:1 " << tileXToLon( 0, 0 ) << ":" << tileYToLat( 1, 0 ) << "\n"
        << "1:1 " << tileXToLon( 1, 0 ) << ":" << tileYToLat( 1, 0 ) << "\n"
        << "Testing for level 1\n"
        << "0:0 " << tileXToLon( 0, 1 ) << ":" << tileYToLat( 0, 1 ) << "\n"
        << "1:0 " << tileXToLon( 1, 1 ) << ":" << tileYToLat( 0, 1 ) << "\n"
        << "2:0 " << tileXToLon( 2, 1 ) << ":" << tileYToLat( 0, 1 ) << "\n"
        << "3:0 " << tileXToLon( 3, 1 ) << ":" << tileYToLat( 0, 1 ) << "\n"
        << "4:0 " << tileXToLon( 4, 1 ) << ":" << tileYToLat( 0, 1 ) << "\n"
        << "0:1 " << tileXToLon( 0, 1 ) << ":" << tileYToLat( 1, 1 ) << "\n"
        << "0:2 " << tileXToLon( 0, 1 ) << ":" << tileYToLat( 2, 1 ) << "\n"
        << "0:3 " << tileXToLon( 0, 1 ) << ":" << tileYToLat( 3, 1 ) << "\n"
        << "0:4 " << tileXToLon( 0, 1 ) << ":" << tileYToLat( 4, 1 ) << "\n"
        << "1:1 " << tileXToLon( 1, 1 ) << ":" << tileYToLat( 1, 1 ) << "\n"
        << "2:2 " << tileXToLon( 2, 1 ) << ":" << tileYToLat( 2, 1 ) << "\n"
        << "3:3 " << tileXToLon( 3, 1 ) << ":" << tileYToLat( 3, 1 ) << "\n"
        << "4:4 " << tileXToLon( 4, 1 ) << ":" << tileYToLat( 4, 1 ) << "\n";

    auto tiles = findTilesToProcess( mapZoomLevel, x, y );
    
    TSP() << "Visible tiles [" << tiles.size() << "]:";

    for ( const auto& head : tiles )
    {
        TSP() << head.z << ", " << head.x << ", " << head.y;
    }
}


int MapGenerator::lonToTileX( double lon, int z ) const
{
    return static_cast<int>( std::floor( ( lon + 180.0 ) / 360.0 * std::pow( 2.0, z ) ) );
}


int MapGenerator::latToTileY( double lat, int z ) const
{
    return static_cast<int>(
        floor( ( 1.0 - log( tan( lat * degToRad ) + 1.0 / cos( lat * degToRad ) ) / pi ) /
            2.0 * pow( 2.0, z ) ) );
}


double MapGenerator::tileXToLon( int x, int z ) const
{
    return x / pow( 2.0, z ) * 360.0 - 180;
}


double MapGenerator::tileYToLat( int y, int z ) const
{
    double n = pi - 2.0 * pi * y / std::pow( 2.0, z );
    return 180.0 / pi * std::atan( 0.5 * ( std::exp( n ) - std::exp( -n ) ) );
}


bool MapGenerator::visiblePoint( double& lon, double& lat )
{
    double x0;
    double x1;
    double y0;
    double y1;
    std::tie( x0, x1, y0, y1 ) = *getViewBorder();
    float unitInMeter = *getUnitInMeter();
    static const float unitLimit = defs::earthRadius * unitInMeter;

    TSP() << "Borders in units:\n"
        << "x: " << x0 << ":" << x1 << "\n"
        << "y: " << y0 << ":" << y1 << "\n"
        << "unitLimit = " << unitLimit;

    if ( unitLimit <= x0 || x1 <= -unitLimit || unitLimit <= y0 || y1 <= -unitLimit )
    {
        return false;
    }

    static const double latLimit = 85.0;    // Safety measure, there are no tiles out of [-85.0511, +85.0511] latitude region
    std::tie( lon, lat ) = *getProjectionCenter();

    if ( x0 < -unitLimit && unitLimit < x1 && y0 < -unitLimit && unitLimit < y1 )
    {
        if ( std::abs( lat ) < latLimit )
        {
            return true;
        }
        else
        {
            lat = lat > 0 ? 85.0 : -85.0;
            return true;
        }
    }

    static const int pixelStep = 10;
    const double meterStep = pixelStep * ( *getMeterInPixel() );
    int pixelW;
    int pixelH;
    std::tie( pixelW, pixelH ) = *getViewPixelSize();
    const int xNum = pixelW / pixelStep;
    const int yNum = pixelH / pixelStep;

    double metX0 = x0 / unitInMeter;
    double metX1 = x1 / unitInMeter;
    double metY0 = y0 / unitInMeter;
    double metY1 = y1 / unitInMeter;
    bool ok;

    for ( int i = 0; i < xNum; ++i )
    {
        std::tie( ok, lon, lat ) = *getInvProjection( metX0 + i * meterStep, metY0 );

        if ( ok )
        {
            return true;
        }

        std::tie( ok, lon, lat ) = *getInvProjection( metX0 + i * meterStep, metY1 );

        if ( ok )
        {
            return true;
        }
    }

    for ( int i = 0; i < yNum; ++i )
    {
        std::tie( ok, lon, lat ) = *getInvProjection( metX0, metY0 + i * meterStep );

        if ( ok )
        {
            return true;
        }

        std::tie( ok, lon, lat ) = *getInvProjection( metX1, metY0 + i * meterStep );

        if ( ok )
        {
            return true;
        }
    }

    return false;
}


std::vector<TileHead> MapGenerator::findTilesToProcess( int z, int x, int y )
{
    std::vector<TileHead> res;
    res.emplace_back( z, x, y );

    std::vector<TileHead> vec[2];
    const int maxInd = std::pow( 2, z ) - 1;
    int curr = 0;
    int next = 0;
    vec[0].emplace_back( z, x, y );

    while ( !vec[next].empty() )
    {
        curr = next;
        next = curr == 0 ? 1 : 0;
        vec[next].clear();

        for ( int i = 0; i < vec[curr].size(); ++i )
        {
            const auto& head = vec[curr][i];
            decltype( res ) adj;

            if ( head.y < maxInd )
            {
                adj.emplace_back( head.z, head.x, head.y + 1 );
            }

            if ( head.y > 0 )
            {
                adj.emplace_back( head.z, head.x, head.y - 1 );
            }

            adj.emplace_back( head.z, head.x == maxInd ? 0 : head.x + 1, head.y );
            adj.emplace_back( head.z, head.x == 0 ? maxInd : head.x - 1, head.y );

            for ( const auto& item : adj )
            {
                if ( std::find( res.begin(), res.end(), item ) != res.end() )
                {
                    continue;
                }

                res.emplace_back( item );

                if ( tileVisible( item ) )
                {
                    vec[next].emplace_back( item );
                }
            }
        }
    }

    return res;
}


bool MapGenerator::tileVisible( TileHead th )
{
    double x0;
    double x1;
    double y0;
    double y1;
    std::tie( x0, x1, y0, y1 ) = *getViewBorder();
    float unitInMeter = *getUnitInMeter();
    x0 /= unitInMeter;
    x1 /= unitInMeter;
    y0 /= unitInMeter;
    y1 /= unitInMeter;

    std::array<TileHead, 4> corners = { {
        { th.z, th.x, th.y } , { th.z, th.x + 1, th.y },
        { th.z, th.x, th.y + 1 }, { th.z, th.x + 1, th.y + 1 }
        } };

    double tx;
    double ty;
    double lon;
    double lat;
    bool ok;

    for ( int i = 0; i < 4; ++i )
    {
        lon = tileXToLon( corners[i].x, corners[i].z );
        lat = tileYToLat( corners[i].y, corners[i].z );
        std::tie( ok, tx, ty ) = *getFwdProjection( lon, lat );

        if ( !ok )
        {
            continue;
        }

        if ( x0 < tx && tx < x1 && y0 < ty && ty < y1 )
        {
            return true;
        }
    }

    return false;
}


}
