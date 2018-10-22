#pragma once

#include <tuple>

#include <boost/signals2.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include "LoadGL.h"
#include "type/ViewData.h"


namespace gv {


class Viewport
{
public:
    Viewport();
    ~Viewport();

    void resize( int w, int h );
    void move( int x, int y );
    void zoom( int steps );
    void center();

    ViewData viewData() const;
    float unitInMeter() const;
    float meterInPixel() const;
    int mapZoomLevel( int tileWidth ) const;
    /// Returns (-x, +x, -y, +y) in gl units
    std::tuple<double, double, double, double> viewBorderUnits() const;
    std::tuple<int, int> viewPixelSize() const;

    glm::mat4 projection() const;

    boost::signals2::signal<void( ViewData )> viewUpdated;

private:
    void setProjection();

    int pixelW_;    //!< Width of viewport (in pixels)
    int pixelH_;    //!< Height of viewport (in pixels)
    float unitX_;   //!< Coordinate x of base point of viewport (in gl units)
    float unitY_;   //!< Coordinate y of base point of viewport (in gl units)
    float unitW_;   //!< Width of viewport (in gl units)
    float unitH_;   //!< Height of viewport (in gl units)
    float panX_;    //!< Pannig by coordinate x (in gl units)
    float panY_;    //!< Pannig by coordinate y (in gl units)

    const float unitInMeter_;   //!< gl units in one meter
    float meterInPixel_;        //!< meters in one pixel
    float unitInPixel_;         //!< gl units in one pixel

    const float maxLen_;            //!< maximum Globe diameter plus 20% of it used as free space
    const float minUnitInPixel_;    //!< minimum value of unitInPixel_
    const float maxUnitInPixel_;    //!< maximum value of unitInPixel_
    const float zoomStep_;          //!< change of unitInPixel_ in one step

    const GLfloat zNear_;
    const GLfloat zFar_;
    const GLfloat zCamera_;
    glm::mat4 proj_;
};


}
