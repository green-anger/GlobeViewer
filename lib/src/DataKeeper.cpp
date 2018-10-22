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

    //////// TEST ///////////////////////////
    /*
    const GLfloat side = 600.0f;
    const std::vector<GLfloat> verts =
    {
        -side, -side, 0.0f, 0.0f,
        +side, -side, 1.0f, 0.0f,
        +side, +side, 1.0f, 1.0f,
        -side, -side, 0.0f, 0.0f,
        +side, +side, 1.0f, 1.0f,
        -side, +side, 0.0f, 1.0f
    };
    numMap_ = 6;

    glBindBuffer( GL_ARRAY_BUFFER, vboMap_ );
    glBufferData( GL_ARRAY_BUFFER, sizeof( GLfloat ) * verts.size(), &verts[0], GL_STATIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    int w;
    int h;
    int chans;
    stbi_set_flip_vertically_on_load( true );

    glBindTexture( GL_TEXTURE_2D, texMap_ );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, 0 );
    stbi_uc* buffer[4] = {
        stbi_load( "cache/1/0/0.png", &w, &h, &chans, 0 ),
        stbi_load( "cache/1/0/1.png", &w, &h, &chans, 0 ),
        stbi_load( "cache/1/1/0.png", &w, &h, &chans, 0 ),
        stbi_load( "cache/1/1/1.png", &w, &h, &chans, 0 )
    };

    TSP() << "Buffer sizes:\n"
        << "buffer[0] = " << strlen( (const char*) buffer[0] ) << "\n"
        << "buffer[1] = " << strlen( (const char*) buffer[1] ) << "\n"
        << "buffer[2] = " << strlen( (const char*) buffer[2] ) << "\n"
        << "buffer[3] = " << strlen( (const char*) buffer[3] ) << "\n";

    //glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, buffer[1] );
    //glTexSubImage2D( GL_TEXTURE_2D, 0, 256, 0, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, buffer[3] );
    //glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 256, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, buffer[0] );
    //glTexSubImage2D( GL_TEXTURE_2D, 0, 256, 256, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, buffer[2] );

    std::vector<unsigned char> vec;
    const std::size_t len = strlen( ( const char* ) buffer[0] );
    const int cols = 2;
    const int chan = 3;
    const std::size_t texW = defs::tileSide * chan * cols;
    vec.resize( 4 * len, 0 );
    for ( int i = 0; i < defs::tileSide; ++i )
        memcpy( &vec[( 0 * defs::tileSide + i ) * texW + 0 * defs::tileSide * chan], &buffer[1][i * defs::tileSide * chan], defs::tileSide * chan );
    for ( int i = 0; i < defs::tileSide; ++i )
        memcpy( &vec[( 0 * defs::tileSide + i ) * texW + 1 * defs::tileSide * chan], &buffer[3][i * defs::tileSide * chan], defs::tileSide * chan );
    for ( int i = 0; i < defs::tileSide; ++i )
        memcpy( &vec[( 1 * defs::tileSide + i ) * texW + 0 * defs::tileSide * chan], &buffer[0][i * defs::tileSide * chan], defs::tileSide * chan );
    for ( int i = 0; i < defs::tileSide; ++i )
        memcpy( &vec[( 1 * defs::tileSide + i ) * texW + 1 * defs::tileSide * chan], &buffer[2][i * defs::tileSide * chan], defs::tileSide * chan );

    //memcpy( &vec[0 * len], buffer[1], len );
    //memcpy( &vec[1 * len], buffer[3], len );
    //memcpy( &vec[2 * len], buffer[0], len );
    //memcpy( &vec[3 * len], buffer[2], len );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, vec.data() );
    glBindTexture( GL_TEXTURE_2D, 0 );

    mapReady( true );
    //*/
    //////// END OF TEST ////////////////////
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


void DataKeeper::updateTexture( std::vector<GLfloat> vecVbo, int w, int h, std::vector<unsigned char> vecData )
{
    glBindTexture( GL_TEXTURE_2D, texMap_ );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, vecData.empty() ? nullptr : vecData.data() );
    glBindTexture( GL_TEXTURE_2D, 0 );

    glBindBuffer( GL_ARRAY_BUFFER, vboMap_ );
    glBufferData( GL_ARRAY_BUFFER, vecVbo.size() * sizeof( GLfloat ), vecVbo.empty() ? nullptr : &vecVbo[0], GL_STATIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    numMap_ = vecVbo.size() / 4;

    mapReady( true );
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
