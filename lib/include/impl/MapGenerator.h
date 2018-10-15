#pragma once

#include <boost/signals2.hpp>


namespace gv {


class MapGenerator
{
public:
    MapGenerator();
    ~MapGenerator();

    void regenerateMap();

    boost::signals2::signal<float()> getUnitInMeter;
    boost::signals2::signal<float()> getMeterInPixel;
    boost::signals2::signal<int()> getMapZoomLevel;
    boost::signals2::signal<std::tuple<double, double, double, double>()> getViewBorder;
    boost::signals2::signal<std::tuple<int, int>()> getViewPixelSize;
    boost::signals2::signal<std::tuple<double, double>()> getProjectionCenter;
    boost::signals2::signal<std::tuple<bool, double, double>( double, double )> getInvProjection;

private:
    bool visiblePoint( double& lon, double& lat );
};


}
