#pragma once

#include <cmath>


/*!
 * \brief Common constants that might be useful.
 */
namespace defs {


const double earthRadius = 6378140.0;   //!< Radius of the Earth.
const double pi = atan( 1.0 ) * 4.0;    //!< Number pi.
const double degToRad = pi / 180.0;     //!< Coefficient to convert degrees to radians.
const double radToDeg = 180.0 / pi;     //!< Coefficient to convert radians to degrees.

const int tileSide = 256;               //!< Map tile side in pixels.


}