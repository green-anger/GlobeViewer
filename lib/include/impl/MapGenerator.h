#pragma once

#include <boost/signals2.hpp>


namespace gv {


class MapGenerator
{
public:
    MapGenerator();
    ~MapGenerator();

    void regenerateMap();

    boost::signals2::signal<int()> getMapZoomLevel;

private:
};


}
