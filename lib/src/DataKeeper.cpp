#include <cmath>
#include <fstream>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

#include <iostream>

//#define GLM_FORCE_RADIANS
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>

#include "DataKeeper.h"
#include "Profiler.h"
#include "Projector.h"


namespace
{

std::mutex mutWire;

}


namespace gv
{


using namespace support;


DataKeeper::DataKeeper()
    : projector_( new Projector )
    , numST_( 0 )
    , numWire_( 0 )
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
}


DataKeeper::~DataKeeper()
{
    std::ofstream file( "profile_map.txt", std::ios::out );
    Profiler::printStatistics( file );
    file.close();
}


void DataKeeper::init()
{
    if ( !unitInMeterGrabber_ )
    {
        throw std::logic_error( "Cannot initialize DataKeeper. Not unitInMeterGrabber defined!" );
    }
    else
    {
        unitInMeter_ = unitInMeterGrabber_();
    }

    if ( !meterInPixelGrabber_ )
    {
        throw std::logic_error( "Cannot initialize DataKeeper. Not meterInPixelGrabber defined!" );
    }

    composeWireGlobe();
}


void DataKeeper::rotateGlobe( int pixelX, int pixelY )
{
    Profiler prof( "DataKeeper::rotateGlobe" );

    float meterInPixel = meterInPixelGrabber_();
    static const double stepLon = 0.05;
    static const double stepLat = 0.05;
    double lon;
    double lat;

    if ( projector_->projectInv( pixelX * meterInPixel, pixelY * meterInPixel, lon, lat ) )
    {
        const auto projLon = projector_->projLon();
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

        const auto projLat = projector_->projLat();
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
            rotatedLat_ = 0.0;
            rotate = true;
        }

        if ( rotate )
        {
            projector_->setProjectionAt( newLon, newLat );
            composeWireGlobe();
        }
    }
}


void DataKeeper::registerUnitInMeterGrabber( const std::function<float()>& func )
{
    unitInMeterGrabber_ = func;
}


void DataKeeper::registerMeterInPixelGrabber( const std::function<float()>& func )
{
    meterInPixelGrabber_ = func;
}


std::tuple<GLuint, std::size_t> DataKeeper::simpleTriangle() const
{
    return std::make_tuple( vaoST_, numST_ );
}


std::tuple<GLuint, std::size_t> DataKeeper::wireGlobe() const
{
    std::lock_guard<std::mutex> lock( mutWire );
    return std::make_tuple( vaoWire_, numWire_ );
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
    static const int lons = std::round( ( endLon - begLon ) / gapLon + 1 );
    static const int lats = std::round( ( endLat - begLat ) / gapLat + 1 );
    static const float drawGapLon = 1.0f;
    static const float drawGapLat = 1.0f;
    static const int drawLons = std::round( ( endLon - begLon ) / drawGapLon + 1 );
    static const int drawLats = std::round( ( endLat - begLat ) / drawGapLat + 1 );

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

    std::lock_guard<std::mutex> lock( mutWire );

    glBindBuffer( GL_ARRAY_BUFFER, vboWire_ );
    glBufferData( GL_ARRAY_BUFFER, vec.size() * sizeof( GLfloat ), vec.empty() ? nullptr : &vec[0], GL_STATIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    numWire_ = vec.size();
}


}
