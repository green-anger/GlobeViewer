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


/*!
 * \param[in] lon Longitude.
 * \param[in] lat Latitude.
 */
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


/*!
 * \param[in] lon Longitude.
 * \param[in] lat Latitude.
 * \param[out] x Projected meters along axis x.
 * \param[out] y Projected meters along axis y.
 * \return
 * True - projecting succeeded.\n
 * False - projecting failed.
 */
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


/*!
 * \param[in] lon Longitude.
 * \param[in] lat Latitude.
 * \return Tuple:\n
 * (0) Result (true - projecting succeeded, false - projecting failed). \n
 * (1) Projected meters along axis x. \n
 * (2) Projected meters along axis y.
 */
std::tuple<bool, double, double> Projector::projectFwd( double lon, double lat ) const
{
    double x;
    double y;
    bool ok = projectFwd( lon, lat, x, y );
    return std::make_tuple( ok, x, y );
}


/*!
 * \param[in] x Projected meters along axis x.
 * \param[in] y Projected meters along axis y.
 * \param[out] lon Longitude.
 * \param[out] lat Latitude.
 * \return
 * True - projecting succeeded.\n
 * False - projecting failed.
 */
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


/*!
 * \param[in] x Projected meters along axis x.
 * \param[in] y Projected meters along axis y.
 * \return
 * (0) Result (true - projecting succeeded, false - projecting failed). \n
 * (1) Longitude. \n
 * (2) Latitude.
 */
std::tuple<bool, double, double> Projector::projectInv( double x, double y ) const
{
    double lon;
    double lat;
    bool ok = projectInv( x, y, lon, lat );
    return std::make_tuple( ok, lon, lat );
}


/*!
 * \param[out] lon Longitude.
 * \param[out] lat Latitude.
 */
void Projector::projectionCenter( double& lon, double& lat ) const
{
    std::lock_guard<std::mutex> lock( mutex_ );
    lon = projLon_;
    lat = projLat_;
}


/*!
 * \return
 * (0) Longitude. \n
 * (1) Latitude.
 */
std::tuple<double, double> Projector::projectionCenter() const
{
    std::lock_guard<std::mutex> lock( mutex_ );
    return std::make_tuple( projLon_, projLat_ );
}


}
