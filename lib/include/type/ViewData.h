#pragma once


namespace gv {


/*!
 * \brief Viewport dimensions and units ratio.
 *
 * It's designed to transfer information about the viewport
 * without giving direct access to it. It helps to keep
 * modules of the system decoupled.
 */
struct ViewData
{
    float unitInMeter;      //!< GL units in one meter.
    float meterInPixel;     //!< Meters in one pixel.
    int mapZoomLevel;       //!< Map zoom level.
    float glX0;             //!< Coordinate x of left-bottom corner in GL units.
    float glX1;             //!< Coordinate x of right-top corner in GL units.
    float glY0;             //!< Coordinate y of left-bottom corner in GL units.
    float glY1;             //!< Coordinate y of right-top corner in GL units.
    int pixWidth;           //!< Width of the viewport in pixels.
    int pixHeight;          //!< Height of the viewport in pixels.
};


}

