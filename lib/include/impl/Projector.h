#pragma once

#include <mutex>
#include <tuple>

#include <proj.h>


namespace gv {


class Projector
{
public:
    Projector();
    ~Projector();

    void setProjectionAt( double lon, double lat );

    bool projectFwd( double lon, double lat, double& x, double& y ) const;
    std::tuple<bool, double, double> projectFwd( double lon, double lat ) const;
    bool projectInv( double x, double y, double& lon, double& lat ) const;
    std::tuple<bool, double, double> projectInv( double x, double y ) const;
    void projectionCenter( double& lon, double& lat ) const;
    std::tuple<double,double> projectionCenter() const;

private:
    PJ_CONTEXT* context_;       //!< PROJ4 Context
    PJ* projection_;            //!< PROJ4 projection

    double projLon_;
    double projLat_;

    mutable std::mutex mutex_;
};


}
