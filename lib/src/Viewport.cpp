#include <algorithm>
#include <cmath>

#include "Defines.h"
#include "Viewport.h"


namespace gv
{


const float Viewport::unitInMeter_ = 0.001f;
const float Viewport::minLen_ = 300.0f;
const float Viewport::maxLen_ = defs::earthRadius * 2 * 3.5f;
const float Viewport::minUnitInPixel_ = minLen_ / 1920 /*pixels*/ * unitInMeter_;
const float Viewport::maxUnitInPixel_ = maxLen_ / 1080 /*pixels*/ * unitInMeter_;
const std::map<float, float> Viewport::zoomMap_ = {
    { unitInMeter_ * 0.2, 0.02 * unitInMeter_ },
    { unitInMeter_ * 0.5, 0.05 * unitInMeter_ },
    { unitInMeter_ * 1.0, 0.1 * unitInMeter_ },
    { unitInMeter_ * 10.0, 0.2 * unitInMeter_ },
    { unitInMeter_ * 50.0, 0.5 * unitInMeter_ },
    { unitInMeter_ * 100.0, 1 * unitInMeter_ },
    { unitInMeter_ * 500.0, 10 * unitInMeter_ },
    { unitInMeter_ * 1000.0, 50 * unitInMeter_ },
    { unitInMeter_ * 2500.0, 125 * unitInMeter_ },
    { unitInMeter_ * 5000.0, 250 * unitInMeter_ },
    { unitInMeter_ * 10000.0, 500 * unitInMeter_ },
    { unitInMeter_ * 20000.0, 700 * unitInMeter_ },
    { unitInMeter_ * 30000.0, 800 * unitInMeter_ },
    { unitInMeter_ * 40000.0, 900 * unitInMeter_ },
    { maxUnitInPixel_, 1000 * unitInMeter_ }
};

Viewport::Viewport()
    : pixelW_( 0 )
    , pixelH_( 0 )
    , unitX_( 0.0f )
    , unitY_( 0.0f )
    , unitW_( 0.0f )
    , unitH_( 0.0f )
    , panX_( 0.0f )
    , panY_( 0.0f )
    , meterInPixel_( 1000.0f )
    , unitInPixel_( meterInPixel_ * unitInMeter_ )
    , zNear_( 0.0f )
    , zFar_( 100.0f )
    , zCamera_( zFar_ )
    , proj_( glm::ortho( -1.0f, 1.0f, -1.0f, 1.0f, zNear_, zFar_ ) )
{
}


Viewport::~Viewport()
{
}


void Viewport::resize( int w, int h )
{
    pixelW_ = w;
    pixelH_ = h;
    const auto prevUnitW = unitW_;
    const auto prevUnitH = unitH_;
    unitW_ = unitInPixel_ * pixelW_;
    unitH_ = unitInPixel_ * pixelH_;
    unitX_ -= ( unitW_ - prevUnitW ) / 2;
    unitY_ -= ( unitH_ - prevUnitH ) / 2;
    setProjection();
    glViewport( 0, 0, w, h );
}


void Viewport::move( int x, int y )
{
    panX_ -= unitInPixel_ * x;
    panY_ -= unitInPixel_ * y;
    setProjection();
}


void Viewport::zoom( int steps )
{
    const auto prevUnitInPixel = unitInPixel_;
    const int sign = steps > 0 ? 1 : -1;

    for ( int i = 0; i < std::abs( steps ); ++i )
    {
        const auto er = zoomMap_.equal_range( unitInPixel_ );
        float diff;

        if ( er.first == er.second )
        {
            diff = er.first->second;
        }
        else if ( sign < 0 && er.second != zoomMap_.end() )
        {
            diff = er.second->second;
        }
        else
        {
            diff = er.first->second;
        }

        unitInPixel_ -= diff * sign;
        unitInPixel_ = std::min( unitInPixel_, maxUnitInPixel_ );
        unitInPixel_ = std::max( unitInPixel_, minUnitInPixel_ );
    }

    if ( prevUnitInPixel == unitInPixel_)
    {
        return;
    }

    meterInPixel_ = unitInPixel_ / unitInMeter_;
    const auto prevUnitW = unitW_;
    const auto prevUnitH = unitH_;
    unitW_ = unitInPixel_ * pixelW_;
    unitH_ = unitInPixel_ * pixelH_;
    unitX_ -= ( unitW_ - prevUnitW ) / 2;
    unitY_ -= ( unitH_ - prevUnitH ) / 2;
    setProjection();
}


void Viewport::center()
{
    panX_ = panY_ = 0;
    setProjection();
}


ViewData Viewport::viewData() const
{
    ViewData vd;
    vd.unitInMeter = unitInMeter_;
    vd.meterInPixel = meterInPixel_;
    vd.mapZoomLevel = mapZoomLevel( defs::tileSide );
    vd.glX0 = unitX_ + panX_;
    vd.glX1 = unitX_ + panX_ + unitW_;
    vd.glY0 = unitY_ + panY_;
    vd.glY1 = unitY_ + panY_ + unitH_;
    vd.pixWidth = pixelW_;
    vd.pixHeight = pixelH_;
    return vd;
}


float Viewport::unitInMeter() const
{
    return unitInMeter_;
}


float Viewport::meterInPixel() const
{
    return meterInPixel_;
}


glm::mat4 Viewport::projection() const
{
    return proj_;
}


std::tuple<double, double> Viewport::metersAtPixel( int x, int y ) const
{
    double metX = ( unitX_ + panX_ + x * unitInPixel_ ) / unitInMeter_;
    double metY = ( unitY_ + panY_ + ( pixelH_ - y ) * unitInPixel_ ) / unitInMeter_;
    return std::make_tuple( metX, metY );
}


void Viewport::setProjection()
{
    static const glm::mat4 view = glm::lookAt(
        glm::vec3( 0.0f, 0.0f, zCamera_ ),  // position
        glm::vec3( 0.0f, 0.0f, 0.0f ),      // target
        glm::vec3( 0.0f, 1.0f, 0.0f )       // up vector
    );
    proj_ = glm::ortho(
        unitX_ + panX_, unitX_ + panX_ + unitW_,
        unitY_ + panY_, unitY_ + panY_ + unitH_,
        zNear_, zFar_ ) *
        view;
    viewUpdated( viewData() );
}


int Viewport::mapZoomLevel( int tileWidth ) const
{
    // 2 * earthRadius - length of projected "front side" of the Earth
    // 2 * earthRadius - length of projected "back side" of the Earth
    // 2^n - number of tiles in one row (or column, the same) for zoom level n
    double tileNum = defs::earthRadius * 4 / meterInPixel_ / tileWidth;

    return static_cast<int>( std::round( std::log2( tileNum ) ) );
}


}
