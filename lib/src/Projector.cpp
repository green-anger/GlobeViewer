﻿#include <cmath>

#include "Projector.h"


namespace gv
{


Projector::Projector()
{
    context_ = proj_context_create();
    projection_ = proj_create( context_, "+proj=ortho +ellps=WGS84 +lon_0=0 +lat_0=0" );
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