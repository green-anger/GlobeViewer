#pragma once

#include <mutex>
#include <tuple>

#include <proj.h>


namespace gv {


/*!
 * \brief Projects the globe to a plane.
 *
 * Projector provides some sort of abstraction of projection process.
 * User works only with degrees (radians) and projected values,
 * usually meters. How projection is handled is no concern of the user.
 * Currently Projector is implemented with help of PROJ.4 library and
 * provides only one projection option (orthographic).
 */
class Projector
{
public:
    Projector();
    ~Projector();

    //! Set projection center at provided coordinates.
    void setProjectionAt( double lon, double lat );

    //! Forward projection converts geographic coordinates to meters.
    bool projectFwd( double lon, double lat, double& x, double& y ) const;
    
    //! Overload of forward projection.
    std::tuple<bool, double, double> projectFwd( double lon, double lat ) const;

    //! Inverted projection converts meters to geographic coordinates.
    bool projectInv( double x, double y, double& lon, double& lat ) const;

    //! Overload of inverted projection.
    std::tuple<bool, double, double> projectInv( double x, double y ) const;

    //! Provide coordinates of projection center.
    void projectionCenter( double& lon, double& lat ) const;

    //! Overload for projectionCenter.
    std::tuple<double,double> projectionCenter() const;

private:
    PJ_CONTEXT* context_;       //!< PROJ.4 Context.
    PJ* projection_;            //!< PROJ.4 projection.

    double projLon_;            //!< Longitude of projection center.
    double projLat_;            //!< Latitude of projection center.

    mutable std::mutex mutex_;  //!< To make API thread safe.
};


}
