#include <cmath>
#include <iostream>

#include "Projector.h"


namespace gv
{


Projector::Projector()
{
    context_ = proj_context_create();
    projection_ = proj_create( context_, "+proj=ortho +ellps=WGS84 +lon_0=0 +lat_0=0" );

    std::cout << "Let's do some projection\n";
    double x;
    double y;

    projectFwd( 90.0f, 0.0f, x, y );
    std::cout << "  90:  0 -> " << x << ":" << y << "\n";
    projectFwd( -90.0f, 0.0f, x, y );
    std::cout << " -90:  0 -> " << x << ":" << y << "\n";
    projectFwd( 0.0f, 90.0f, x, y );
    std::cout << "   0: 90 -> " << x << ":" << y << "\n";
    projectFwd( 0.0f, -90.0f, x, y );
    std::cout << "   0:-90 -> " << x << ":" << y << "\n";

    const double tmp = 6.37814e+06;
    const double tmp2 = 6378140.0;
    std::cout << "tmp  = " << tmp << std::endl;
    std::cout << "tmp2 = " << tmp2 << std::endl;
    std::cout << std::endl;
}


Projector::~Projector()
{
    proj_destroy( projection_ );
    proj_context_destroy( context_ );
}


bool Projector::projectFwd( double lon, double lat, double& x, double& y ) const
{
    PJ_COORD src = proj_coord( proj_torad( lon ), proj_torad( lat ), 0.0, 0.0 );
    PJ_COORD dst = proj_trans( projection_, PJ_FWD, src );
    x = dst.xy.x;
    y = dst.xy.y;

    return ( x != HUGE_VAL && y != HUGE_VAL );
}


bool Projector::projectInv( double x, double y, double& lon, double& lat ) const
{
    PJ_COORD src = proj_coord( proj_torad( lon ), proj_torad( lat ), 0.0, 0.0 );
    PJ_COORD dst = proj_trans( projection_, PJ_FWD, src );
    x = dst.xy.x;
    y = dst.xy.y;

    return ( x != HUGE_VAL && y != HUGE_VAL );
}


}
