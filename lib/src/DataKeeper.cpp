#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "DataKeeper.h"
#include "Defines.h"
#include "Profiler.h"
#include "Projector.h"
#include "stb_image.h"
#include "ThreadSafePrinter.hpp"


using TSP = alt::ThreadSafePrinter<alt::MarkPolicy>;


namespace gv
{


using namespace support;


DataKeeper::DataKeeper()
    : numST_( 0 )
    , numWire_( 0 )
    , numMap_( 0 )
    , rotatedLon_( 0.0 )
    , rotatedLat_( 0.0 )
{
    const GLfloat side = 100.0f;
    const std::vector<GLfloat> stVerts =
    {
        -side, -side,
        +side, -side,
         0.0f, +side
    };
    numST_ = 3;

    glGenVertexArrays( 1, &vaoST_ );
    glGenBuffers( 1, &vboST_ );
    glBindVertexArray( vaoST_ );
    glBindBuffer( GL_ARRAY_BUFFER, vboST_ );
    glBufferData( GL_ARRAY_BUFFER, sizeof( GLfloat ) * stVerts.size(), &stVerts[0], GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( GLfloat ), ( void* )0 );
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

    glGenVertexArrays( 1, &vaoWire_ );
    glGenBuffers( 1, &vboWire_ );
    glBindVertexArray( vaoWire_ );
    glBindBuffer( GL_ARRAY_BUFFER, vboWire_ );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( GLfloat ), ( void* ) 0 );
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

    glGenVertexArrays( 1, &vaoMap_ );
    glGenBuffers( 1, &vboMap_ );
    glBindVertexArray( vaoMap_ );
    glBindBuffer( GL_ARRAY_BUFFER, vboMap_ );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof( GLfloat ), ( void* ) 0 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof( GLfloat ), ( void* ) ( 2 * sizeof( GLfloat ) ) );
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

    glGenTextures( 1, &texMap_ );
    glBindTexture( GL_TEXTURE_2D, texMap_ );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glBindTexture( GL_TEXTURE_2D, 0 );
}


DataKeeper::~DataKeeper()
{
}


void DataKeeper::init()
{
    boost::optional<float> optUnitInMeter = getUnitInMeter();

    if ( !optUnitInMeter )
    {
        throw std::logic_error( "Cannot initialize DataKeeper: getUnitInMeter is not defined!" );
    }
    else
    {
        unitInMeter_ = *optUnitInMeter;
    }

    if ( !getMeterInPixel() )
    {
        throw std::logic_error( "Cannot initialize DataKeeper: getMeterInPixel is not defined!" );
    }

    if ( !getProjector() )
    {
        throw std::logic_error( "Cannot initialize DataKeeper: getProjector is not defined!" );
    }
    else
    {
        projector_ = *getProjector();
    }

    composeWireGlobe();
}


void DataKeeper::rotateGlobe( int pixelX, int pixelY )
{
    Profiler prof( "DataKeeper::rotateGlobe" );

    float meterInPixel = *getMeterInPixel();
    static const double stepLon = 0.05;
    static const double stepLat = 0.05;
    double lon;
    double lat;

    if ( projector_->projectInv( pixelX * meterInPixel, pixelY * meterInPixel, lon, lat ) )
    {
        double projLon;
        double projLat;
        projector_->projectionCenter( projLon, projLat );

        const int signLon = pixelX < 0 ? 1 : -1;
        double diffLon = abs( lon - projLon );
        
        if ( 180.0 < diffLon )
        {
            diffLon = 360.0 - diffLon;
        }

        rotatedLon_ += signLon * diffLon;
        const double closeLon = rotatedLon_ > 0.0
            ? floor( rotatedLon_ / stepLon ) * stepLon
            : ceil( rotatedLon_ / stepLon ) * stepLon;

        const int signLat = pixelY < 0 ? 1 : -1;
        double diffLat = abs( lat - projLat );

        rotatedLat_ += signLat * diffLat;
        const double closeLat = rotatedLat_ > 0.0
            ? floor( rotatedLat_ / stepLat ) * stepLat
            : ceil( rotatedLat_ / stepLat ) * stepLat;

        double newLon = lon;
        double newLat = lat;
        bool rotate = false;

        if ( stepLon <= abs( closeLon ) )
        {
            newLon = projLon + closeLon;
            rotatedLon_ = 0.0;
            rotate = true;
        }

        if ( stepLat <= abs( closeLat ) )
        {
            newLat = projLat + closeLat;
            newLat = std::min( newLat, +90.0 );
            newLat = std::max( newLat, -90.0 );
            rotatedLat_ = 0.0;
            rotate = true;
        }

        if ( rotate )
        {
            projector_->setProjectionAt( newLon, newLat );
            composeWireGlobe();
            globeRotated();
        }
    }
}


void DataKeeper::balanceGlobe()
{
    projector_->setProjectionAt( 0.0, 0.0 );
    composeWireGlobe();
    globeRotated();
}


void DataKeeper::newTileTexture( const TileTexture& tt )
{
    Profiler prof( "DataKeeper::newTileTexture" );

    static const int chunks = 10;

    std::vector<GLfloat> vec;
    vec.reserve( 24 * tt.tileCount * chunks );

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
                        x[n] = static_cast<GLfloat>( px * unitInMeter_ );
                        y[n] = static_cast<GLfloat>( py * unitInMeter_ );
                    }
                }

                //*
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

                    std::move( vecChunk.begin(), vecChunk.end(), std::back_inserter( vec ) );
                }
                //*/
            }
        }
    }

    //*
    numMap_ = vec.size() / 4;
    tileTexture_ = tt;

    int sideX;
    int sideY;
    std::tie( sideX, sideY ) = tt.textureSize;

    glBindTexture( GL_TEXTURE_2D, texMap_ );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, sideX, sideY, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr );
    glBindTexture( GL_TEXTURE_2D, 0 );

    glBindBuffer( GL_ARRAY_BUFFER, vboMap_ );
    glBufferData( GL_ARRAY_BUFFER, vec.size() * sizeof( GLfloat ), vec.empty() ? nullptr : &vec[0], GL_STATIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    //*/

    //TSP() << "DataKeeper::newTileTexture\n"
    //    << "tileCount = " << tt.tileCount << "\n"
    //    << "vec.size() = " << vec.size();
}


void DataKeeper::updateTexture( const std::vector<TileImage>& vec )
{
    Profiler prof( "DataKeeper::updateTexture" );

    //TSP() << "DataKeeper::updateTexture got " << vec.size() << " tile images";

    if ( tileTexture_.tiles.empty() || vec.empty() )
    {
        return;
    }


    glBindTexture( GL_TEXTURE_2D, texMap_ );
    int width;
    int height;
    int channels;
    stbi_set_flip_vertically_on_load( true );
    const auto sideX = std::get<0>( tileTexture_.textureSize );
    const auto sideY = std::get<1>( tileTexture_.textureSize );

    for ( const auto& tileImg : vec )
    {
        const auto it = tileTexture_.tiles.find( tileImg.head );

        if ( it == tileTexture_.tiles.end() )
        {
            continue;
        }

        const auto& body = it->second;
        const auto& data = tileImg.data.data;
        auto buffer = stbi_load_from_memory( &data[0], data.size(), &width, &height, &channels, 0 );
        glTexSubImage2D( GL_TEXTURE_2D, 0, body.tx0 * sideX, body.ty0 * sideY,
            defs::tileSide, defs::tileSide, GL_RGB, GL_UNSIGNED_BYTE, buffer );
        stbi_image_free( buffer );
    }

    glBindTexture( GL_TEXTURE_2D, 0 );

    /*
    if ( vec.empty() )
    {
        return;
    }

    auto it = std::find_if( vec.begin(), vec.end(), []( const auto& img )
    {
        return img.head == TileHead( 0, 0, 0 );
    } );

    std::vector<unsigned char> data;

    if ( it != vec.end() )
    {
        data = it->data.data;
    }
    else
    {
        data = vec[0].data.data;
    }

    { // filling vbo
        std::vector<GLfloat> vec;
        vec.reserve( 24 );
        const GLfloat len = 400.0f;
        vec = {
            -len, -len, 0.0f, 0.0f,
            +len, -len, 1.0f, 0.0f,
            +len, +len, 1.0f, 1.0f,
            -len, -len, 0.0f, 0.0f,
            +len, +len, 1.0f, 1.0f,
            -len, +len, 0.0f, 1.0f
        };
        numMap_ = 6;

        glBindBuffer( GL_ARRAY_BUFFER, vboMap_ );
        glBufferData( GL_ARRAY_BUFFER, vec.size() * sizeof( GLfloat ), &vec[0], GL_STATIC_DRAW );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
    }

    int width;
    int height;
    int nrChannels;
    stbi_set_flip_vertically_on_load( true );
    auto buffer = stbi_load_from_memory( &data[0], data.size(), &width, &height, &nrChannels, 0 );
    glBindTexture( GL_TEXTURE_2D, texMap_ );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr );
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, buffer );
    glBindTexture( GL_TEXTURE_2D, 0 );
    stbi_image_free( buffer );
    //*/
}


std::tuple<GLuint, GLsizei> DataKeeper::simpleTriangle() const
{
    return std::make_tuple( vaoST_, numST_ );
}


std::tuple<GLuint, GLsizei> DataKeeper::wireGlobe() const
{
    return std::make_tuple( vaoWire_, numWire_ );
}


std::tuple<GLuint, GLuint, GLsizei> DataKeeper::mapTiles() const
{
    return std::make_tuple( vaoMap_, texMap_, numMap_ );
}


void DataKeeper::composeWireGlobe()
{
    Profiler prof( "DataKeeper::composeWireGlobe" );

    static const float gapLon = 10.0f;
    static const float gapLat = 10.0f;
    static const float begLon = -180.0f;
    static const float endLon = 180.0f;
    static const float begLat = -80.0f;
    static const float endLat = 80.0f;
    static const int lons = static_cast<int>( std::round( ( endLon - begLon ) / gapLon + 1 ) );
    static const int lats = static_cast<int>( std::round( ( endLat - begLat ) / gapLat + 1 ) );
    static const float drawGapLon = 1.0f;
    static const float drawGapLat = 1.0f;
    static const int drawLons = static_cast<int>( std::round( ( endLon - begLon ) / drawGapLon + 1 ) );
    static const int drawLats = static_cast<int>( std::round( ( endLat - begLat ) / drawGapLat + 1 ) );

    std::vector<GLfloat> vec;

    for ( int iLat = 0; iLat < lats; ++iLat )
    {
        for ( int iLon = 0; iLon < drawLons - 1; ++iLon )
        {
            double x1;
            double y1;

            if ( !projector_->projectFwd( begLon + iLon * drawGapLon, begLat + iLat * gapLat, x1, y1 ) )
            {
                continue;
            }

            double x2;
            double y2;

            if ( !projector_->projectFwd( begLon + ( iLon + 1 )* drawGapLon, begLat + iLat * gapLat, x2, y2 ) )
            {
                continue;
            }

            vec.emplace_back( static_cast<GLfloat>( x1 * unitInMeter_ ) );
            vec.emplace_back( static_cast<GLfloat>( y1 * unitInMeter_ ) );
            vec.emplace_back( static_cast<GLfloat>( x2 * unitInMeter_ ) );
            vec.emplace_back( static_cast<GLfloat>( y2 * unitInMeter_ ) );
        }
    }

    for ( int iLon = 0; iLon < lons; ++iLon )
    {
        for ( int iLat = 0; iLat < drawLats - 1; ++iLat )
        {
            double x1;
            double y1;

            if ( !projector_->projectFwd( begLon + iLon * gapLon, begLat + iLat * drawGapLat, x1, y1 ) )
            {
                continue;
            }

            double x2;
            double y2;

            if ( !projector_->projectFwd( begLon + iLon * gapLon, begLat + ( iLat + 1 )* drawGapLat, x2, y2 ) )
            {
                continue;
            }

            vec.emplace_back( static_cast<GLfloat>( x1 * unitInMeter_ ) );
            vec.emplace_back( static_cast<GLfloat>( y1 * unitInMeter_ ) );
            vec.emplace_back( static_cast<GLfloat>( x2 * unitInMeter_ ) );
            vec.emplace_back( static_cast<GLfloat>( y2 * unitInMeter_ ) );
        }
    }

    glBindBuffer( GL_ARRAY_BUFFER, vboWire_ );
    glBufferData( GL_ARRAY_BUFFER, vec.size() * sizeof( GLfloat ), vec.empty() ? nullptr : &vec[0], GL_STATIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    numWire_ = vec.size();
}


}
