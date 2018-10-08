#pragma once

#include <mutex>

#include <proj.h>


namespace gv {


class Projector
{
public:
    Projector();
    ~Projector();

    void setProjectionAt( double lon, double lat );

    bool projectFwd( double lon, double lat, double& x, double& y ) const;
    bool projectInv( double x, double y, double& lon, double& lat ) const;
    double projLon() const;
    double projLat() const;

private:
    PJ_CONTEXT* context_;       //!< PROJ4 Context
    PJ* projection_;            //!< PROJ4 projection

    double projLon_;
    double projLat_;

    mutable std::mutex mutex_;
};


}
