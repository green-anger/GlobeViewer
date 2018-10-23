#include <algorithm>
#include <cmath>

#include "Defines.h"
#include "LoadGL.h"
#include "MapGenerator.h"
#include "Profiler.h"
#include "Projector.h"
#include "stb_image.h"
#include "ThreadSafePrinter.hpp"


using TSP = alt::ThreadSafePrinter<alt::MarkPolicy>;
using namespace defs;


namespace gv {


using namespace support;


MapGenerator::MapGenerator()
    : ioc_()
    , work_( make_work_guard( ioc_ ) )
    , active_( false )
    , pending_( false )
    , gotTiles_( false )
    , calcedVbo_( false )
{
    threads_.emplace_back( [this]() { ioc_.run(); } );
}


MapGenerator::~MapGenerator()
{
    work_.reset();
    ioc_.stop();

    for ( auto&& t : threads_ )
    {
        if ( t.joinable() )
        {
            t.join();
        }
    }
}


void MapGenerator::init( ViewData vd )
{
    if ( !getProjector() )
    {
        throw std::logic_error( "Cannot initialize MapGenerator: getProjector is not defined!" );
    }
    else
    {
        projector_ = *getProjector();
    }

    newViewData_ = vd;
    viewData_ = vd;
}


void MapGenerator::updateGlobe()
{
    mapReady( false );
    checkStates();
}


void MapGenerator::updateViewData( ViewData vd )
{
    newViewData_ = vd;
    checkStates();
}


void MapGenerator::getTiles( const std::vector<TileImage>& vec )
{
    data_.clear();
    
    if ( vec.empty() )
    {
        return;
    }
    
    int rowNum;
    int colNum;
    std::tie( colNum, rowNum ) = tileTex_.textureSize;
    const int channels = 3;
    const int tileW = defs::tileSide * channels;
    const int texW = tileW * colNum;

    data_.resize( defs::tileSide * tileW * rowNum * colNum, 0 );

    const auto& tiles = tileTex_.tiles;
    int w;
    int h;
    int chans;
    stbi_set_flip_vertically_on_load( true );

    for ( const auto& ti : vec )
    {
        auto it = tiles.find( ti.head );

        if ( it != tiles.end() )
        {
            const auto& row = it->second.row;
            const auto& col = it->second.col;

            auto buffer = stbi_load_from_memory( &ti.data.data[0], ti.data.data.size(), &w, &h, &chans, 0 );
            
            for ( int i = 0; i < defs::tileSide; ++i )
            {
                memcpy( &data_[( row * defs::tileSide + i ) * texW + col * tileW], &buffer[i * tileW], tileW );
            }
            
            stbi_image_free( buffer );
        }
    }

    gotTiles_.store( true );
    finalize();
}


void MapGenerator::checkStates()
{
    std::lock_guard<std::mutex> lock( mutexState_ );

    if ( active_ )
    {
        pending_ = true;
        return;
    }
    else
    {
        active_ = true;
        ioc_.post( [this] { regenerateMap(); } );
    }
}


void MapGenerator::cleanupCheck()
{
    std::lock_guard<std::mutex> lock( mutexState_ );

    if ( pending_ )
    {
        pending_ = false;
        ioc_.post( [this] { regenerateMap(); } );
    }
    else
    {
        int w = std::get<0>( tileTex_.textureSize ) * defs::tileSide;
        int h = std::get<1>( tileTex_.textureSize ) * defs::tileSide;
        updateMapTexture( vbo_, w, h, data_ );
        active_ = false;
    }
}


void MapGenerator::regenerateMap()
{
    Profiler prof( "MapGenerator::regenerateMap" );

    viewData_ = newViewData_;

    double lon;
    double lat;

    if ( !visiblePoint( lon, lat ) )
    {
        cleanupCheck();
        return;
    }

    int mapZoomLevel = viewData_.mapZoomLevel;
    int x = lonToTileX( lon, mapZoomLevel );
    int y = latToTileY( lat, mapZoomLevel );

    auto tiles = findTilesToProcess( mapZoomLevel, x, y );

    composeTileTexture( tiles );
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
    Profiler prof( "MapGenerator::visiblePoint" );

    double x0 = viewData_.glX0;
    double x1 = viewData_.glX1;
    double y0 = viewData_.glY0;
    double y1 = viewData_.glY1;
    float unitInMeter = viewData_.unitInMeter;
    static const float unitLimit = static_cast<float>( defs::earthRadius * unitInMeter );

    if ( unitLimit <= x0 || x1 <= -unitLimit || unitLimit <= y0 || y1 <= -unitLimit )
    {
        return false;
    }

    static const double latLimit = 85.0;    // Safety measure, there are no tiles out of [-85.0511, +85.0511] latitude region
    projector_->projectionCenter( lon, lat );

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
    const double meterStep = pixelStep * viewData_.meterInPixel;
    int pixelW = viewData_.pixWidth;
    int pixelH = viewData_.pixHeight;
    const int xNum = pixelW / pixelStep;
    const int yNum = pixelH / pixelStep;

    double metX0 = x0 / unitInMeter;
    double metX1 = x1 / unitInMeter;
    double metY0 = y0 / unitInMeter;
    double metY1 = y1 / unitInMeter;

    for ( int i = 0; i < xNum; ++i )
    {
        if ( projector_->projectInv( metX0 + i * meterStep, metY0, lon, lat ) )
        {
            return true;
        }

        if ( projector_->projectInv( metX0 + i * meterStep, metY1, lon, lat ) )
        {
            return true;
        }
    }

    for ( int i = 0; i < yNum; ++i )
    {
        if ( projector_->projectInv( metX0, metY0 + i * meterStep, lon, lat ) )
        {
            return true;
        }

        if ( projector_->projectInv( metX1, metY0 + i * meterStep, lon, lat ) )
        {
            return true;
        }
    }

    return false;
}


std::vector<TileHead> MapGenerator::findTilesToProcess( int z, int x, int y )
{
    Profiler prof( "MapGenerator::findTilesToProcess" );

    std::vector<TileHead> res;
    res.emplace_back( z, x, y );

    std::vector<TileHead> vec[2];
    const int maxInd = static_cast<int>( std::pow( 2, z ) - 1 );
    int curr = 0;
    int next = 0;
    vec[0].emplace_back( z, x, y );

    while ( !vec[next].empty() )
    {
        curr = next;
        next = curr == 0 ? 1 : 0;
        vec[next].clear();

        for ( std::size_t i = 0; i < vec[curr].size(); ++i )
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
    Profiler prof( "MapGenerator::tileVisible" );

    double x0 = viewData_.glX0;
    double x1 = viewData_.glX1;
    double y0 = viewData_.glY0;
    double y1 = viewData_.glY1;
    float unitInMeter = viewData_.unitInMeter;
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

    for ( int i = 0; i < 4; ++i )
    {
        lon = tileXToLon( corners[i].x, corners[i].z );
        lat = tileYToLat( corners[i].y, corners[i].z );

        if ( !projector_->projectFwd( lon, lat, tx, ty ) )
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


void MapGenerator::composeTileTexture( const std::vector<TileHead>& vec )
{
    Profiler prof( "MapGenerator::composeTileTexture" );

    tileTex_.tileCount = vec.size();
    tileTex_.tileFilled = 0;
    const int numX = static_cast<int>( std::ceil( std::sqrt( tileTex_.tileCount ) ) );
    const int numY = static_cast<int>( std::ceil( static_cast<double>( tileTex_.tileCount ) / numX ) );
    const int sideX = numX * defs::tileSide;
    const int sideY = numY * defs::tileSide;
    tileTex_.textureSize = std::make_tuple( numX, numY );
    tileTex_.tiles.clear();

    int row = 0;
    int col = 0;
    std::vector<TileHead> tileHeads;

    for ( const auto& head : vec )
    {
        tileHeads.emplace_back( head );
        TileBody body;
        body.lon0 = tileXToLon( head.x, head.z );
        body.lon1 = tileXToLon( head.x + 1, head.z );
        body.lat0 = tileYToLat( head.y + 1, head.z );
        body.lat1 = tileYToLat( head.y, head.z );
        const float tx = static_cast<float>( col * defs::tileSide );
        const float ty = static_cast<float>( row * defs::tileSide );
        body.tx0 = tx / static_cast<float>( sideX );
        body.tx1 = ( tx + defs::tileSide ) / static_cast<float>( sideX );
        body.ty0 = ty / static_cast<float>( sideY );
        body.ty1 = ( ty + defs::tileSide ) / static_cast<float>( sideY );
        body.row = row;
        body.col = col;
        tileTex_.tiles.emplace( std::move( head ), std::move( body ) );

        if ( ++col == numX )
        {
            col= 0;
            ++row;
        }
    }

    requestTiles( tileHeads );
    vboFromTileTexture( tileTex_ );
}


void MapGenerator::vboFromTileTexture( TileTexture tt )
{
    static const int chunks = 10;

    vbo_.clear();
    vbo_.reserve( 24 * tt.tileCount * chunks );

    for ( const auto& tile : tt.tiles )
    {
        const auto& body = tile.second;
        const double chunkLon = ( body.lon1 - body.lon0 ) / chunks;
        const double chunkLat = ( body.lat1 - body.lat0 ) / chunks;
        const float chunkTx = ( body.tx1 - body.tx0 ) / chunks;
        const float chunkTy = ( body.ty1 - body.ty0 ) / chunks;

        for ( int i = 0; i < chunks; ++i )
        {
            double lon[4];
            lon[0] = body.lon0 + i * chunkLon;
            lon[1] = body.lon0 + ( i + 1 ) * chunkLon;
            lon[2] = lon[1];
            lon[3] = lon[0];
            float tx[4];
            tx[0] = body.tx0 + i * chunkTx;
            tx[1] = body.tx0 + ( i + 1 )* chunkTx;
            tx[2] = tx[1];
            tx[3] = tx[0];

            for ( int j = 0; j < chunks; ++j )
            {
                double lat[4];
                lat[0] = body.lat0 + j * chunkLat;
                lat[1] = lat[0];
                lat[2] = body.lat0 + ( j + 1 ) * chunkLat;
                lat[3] = lat[2];
                float ty[4];
                ty[0] = body.ty0 + j * chunkTy;
                ty[1] = ty[0];
                ty[2] = body.ty0 + ( j + 1 ) * chunkTy;
                ty[3] = ty[2];

                std::vector<int> ind;
                GLfloat x[4];
                GLfloat y[4];
                double px;  // projected x
                double py;  // projected y

                for ( int n = 0; n < 4; ++n )
                {
                    if ( projector_->projectFwd( lon[n], lat[n], px, py ) )
                    {
                        ind.emplace_back( n );
                        x[n] = static_cast<GLfloat>( px * viewData_.unitInMeter );
                        y[n] = static_cast<GLfloat>( py * viewData_.unitInMeter );
                    }
                }

                if ( ind.size() > 2 )
                {
                    std::vector<GLfloat> vecChunk = {
                        x[ind[0]], y[ind[0]], tx[ind[0]], ty[ind[0]],
                        x[ind[1]], y[ind[1]], tx[ind[1]], ty[ind[1]],
                        x[ind[2]], y[ind[2]], tx[ind[2]], ty[ind[2]]
                    };

                    if ( ind.size() == 4 )
                    {
                        vecChunk.insert( vecChunk.end(), {
                            x[ind[0]], y[ind[0]], tx[ind[0]], ty[ind[0]],
                            x[ind[2]], y[ind[2]], tx[ind[2]], ty[ind[2]],
                            x[ind[3]], y[ind[3]], tx[ind[3]], ty[ind[3]] }
                        );
                    }

                    std::move( vecChunk.begin(), vecChunk.end(), std::back_inserter( vbo_ ) );
                }
            }
        }
    }

    calcedVbo_.store( true );
    finalize();
}


void MapGenerator::finalize()
{
    if ( gotTiles_.load() && calcedVbo_.load() )
    {
        calcedVbo_.store( false );
        gotTiles_.store( false );

        cleanupCheck();
    }
}


}
