#include <algorithm>

#include "Defines.h"
#include "MapGenerator.h"
#include "ThreadSafePrinter.hpp"


using TSP = alt::ThreadSafePrinter<alt::MarkPolicy>;


namespace gv {


MapGenerator::MapGenerator()
{
}


MapGenerator::~MapGenerator()
{
}


void MapGenerator::regenerateMap()
{
    double lon;
    double lat;

    if ( !visiblePoint( lon, lat ) )
    {
        TSP() << "No visible point!";
        return;
    }

    TSP() << "Visible point at: " << lon << ":" << lat;

    int mapZoomLevel = *getMapZoomLevel();
}


bool MapGenerator::visiblePoint( double& lon, double& lat )
{
    double x0;
    double x1;
    double y0;
    double y1;
    std::tie( x0, x1, y0, y1 ) = *getViewBorder();
    float unitInMeter = *getUnitInMeter();
    static const float unitLimit = defs::earthRadius * unitInMeter;

    TSP() << "Borders in units:\n"
        << "x: " << x0 << ":" << x1 << "\n"
        << "y: " << y0 << ":" << y1 << "\n"
        << "unitLimit = " << unitLimit;

    if ( unitLimit <= x0 || x1 <= -unitLimit || unitLimit <= y0 || y1 <= -unitLimit )
    {
        return false;
    }

    static const double latLimit = 85.0;    // Safety measure, there are no tiles out of [-85.0511, +85.0511] latitude region
    std::tie( lon, lat ) = *getProjectionCenter();

    if ( x0 < -unitLimit && unitLimit < x1 && y0 < -unitLimit && unitLimit < y1 )
    {
        if ( std::abs( lat ) < latLimit )
        {
            return true;
        }
        else
        {
            lat = lat > 0 ? 85.0 : -85.0;
            return true;
        }
    }

    static const int pixelStep = 10;
    const double meterStep = pixelStep * ( *getMeterInPixel() );
    int pixelW;
    int pixelH;
    std::tie( pixelW, pixelH ) = *getViewPixelSize();
    const int xNum = pixelW / pixelStep;
    const int yNum = pixelH / pixelStep;

    double metX0 = x0 / unitInMeter;
    double metX1 = x1 / unitInMeter;
    double metY0 = y0 / unitInMeter;
    double metY1 = y1 / unitInMeter;
    bool ok;

    for ( int i = 0; i < xNum; ++i )
    {
        std::tie( ok, lon, lat ) = *getInvProjection( metX0 + i * meterStep, metY0 );

        if ( ok )
        {
            return true;
        }

        std::tie( ok, lon, lat ) = *getInvProjection( metX0 + i * meterStep, metY1 );

        if ( ok )
        {
            return true;
        }
    }

    for ( int i = 0; i < yNum; ++i )
    {
        std::tie( ok, lon, lat ) = *getInvProjection( metX0, metY0 + i * meterStep );

        if ( ok )
        {
            return true;
        }

        std::tie( ok, lon, lat ) = *getInvProjection( metX1, metY0 + i * meterStep );

        if ( ok )
        {
            return true;
        }
    }

    return false;
}


}
