#include <algorithm>
#include <iostream>

#include "Viewport.h"


namespace gv
{


Viewport::Viewport()
    : pixelW_( 0 )
    , pixelH_( 0 )
    , unitX_( 0.0f )
    , unitY_( 0.0f )
    , unitW_( 0.0f )
    , unitH_( 0.0f )
    , panX_( 0.0f )
    , panY_( 0.0f )
    , unitInMeter_( 0.001f ) /* const */
    , meterInPixel_( 1000.0f )
    , unitInPixel_( meterInPixel_ * unitInMeter_ )
    , maxOrthoRad_( 6378140.0f ) /* const */
    , maxLen_( maxOrthoRad_ * 2 * 1.2f ) /* const */
    , minUnitInPixel_( 1.0f ) /* const */
    , maxUnitInPixel_( maxLen_ / 1080 /*pixels*/ * unitInMeter_ ) /* const */
    , zoomStep_( 1.0f ) /* const */
    , zNear_( 0.0f )
    , zFar_( 100.0f )
    , zCamera_( zFar_ )
    , proj_( glm::ortho( -1.0f, 1.0f, -1.0f, 1.0f, zNear_, zFar_ ) )
{
    std::cout << "Viewport\n"
        << "unitInPixel_ = " << unitInPixel_ << "\n"
        << "maxLen_ = " << maxLen_ << "\n"
        << "maxUnitInPixel_ = " << maxUnitInPixel_ << "\n"
        << std::endl;
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
    const int sign = steps > 0 ? 1 : -1;

    for ( int i = 0; i < std::abs( steps ); ++i )
    {
        unitInPixel_ -= zoomStep_ * sign;
    }

    unitInPixel_ = std::min( unitInPixel_, maxUnitInPixel_ );
    unitInPixel_ = std::max( unitInPixel_, minUnitInPixel_ );
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


float Viewport::unitInMeter() const
{
    return unitInMeter_;
}


glm::mat4 Viewport::projection() const
{
    return proj_;
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
}


void Viewport::testPrint() const
{
    std::cout
        << "pixelW_ = " << pixelW_ << "\n"
        << "pixelH_ = " << pixelH_ << "\n"
        << "unitW_ = " << unitW_ << "\n"
        << "unitH_ = " << unitH_ << "\n"
        << "unitX_ = " << unitX_ << "\n"
        << "unitY_ = " << unitY_ << "\n"
        << "unitInPixel_ = " << unitInPixel_ << "\n"
        << std::endl;
}


}
