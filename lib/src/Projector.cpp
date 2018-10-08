#include <algorithm>
#include <cmath>
#include <string>

#include "Projector.h"


namespace gv
{


Projector::Projector()
    : projection_( nullptr )
{
    context_ = proj_context_create();
    setProjectionAt( 0.0, 0.0 );
    projLon_ = 0.0;
    projLat_ = 0.0;
}


Projector::~Projector()
{
    proj_destroy( projection_ );
    proj_context_destroy( context_ );
}


void Projector::setProjectionAt( double lon, double lat )
{
    std::lock_guard<std::mutex> lock( mutex_ );

    if ( projection_ )
    {
        proj_destroy( projection_ );
    }

    std::string strLon = std::to_string( lon );
    std::replace( strLon.begin(), strLon.end(), ',', '.' );
    std::string strLat = std::to_string( lat );
    std::replace( strLat.begin(), strLat.end(), ',', '.' );
    std::string strProj = "+proj=ortho +ellps=WGS84 +lon_0=" + strLon + " +lat_0=" + strLat;
    projection_ = proj_create( context_, strProj.c_str() );

    if ( lon < -180.0 ) projLon_ = lon + 360.0;
    else if ( lon >= 180.0 ) projLon_ = lon - 360.0;
    else projLon_ = lon;

    projLat_ = lat;
}


bool Projector::projectFwd( double lon, double lat, double& x, double& y ) const
{
    PJ_COORD res;
    {
        std::lock_guard<std::mutex> lock( mutex_ );
        PJ_COORD src = proj_coord( proj_torad( lon ), proj_torad( lat ), 0.0, 0.0 );
        res = proj_trans( projection_, PJ_FWD, src );
    }
    x = res.xy.x;
    y = res.xy.y;

    return ( x != HUGE_VAL && y != HUGE_VAL );
}


bool Projector::projectInv( double x, double y, double& lon, double& lat ) const
{
    PJ_COORD res;
    {
        std::lock_guard<std::mutex> lock( mutex_ );
        PJ_COORD src = proj_coord( x, y, 0.0, 0.0 );
        res = proj_trans( projection_, PJ_INV, src );
    }
    lon = proj_todeg( res.lp.lam );
    lat = proj_todeg( res.lp.phi );

    return ( lon != HUGE_VAL && lat != HUGE_VAL );
}


double Projector::projLon() const
{
    std::lock_guard<std::mutex> lock( mutex_ );

    return projLon_;
}


double Projector::projLat() const
{
    std::lock_guard<std::mutex> lock( mutex_ );

    return projLat_;
}


}
