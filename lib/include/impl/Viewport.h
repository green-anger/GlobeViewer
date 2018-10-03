#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "LoadGL.h"


namespace gv {


class Viewport
{
public:
    Viewport();
    ~Viewport();

    void resize( int w, int h );
    void move( int x, int y );
    void zoom( int steps );

    glm::mat4 projection() const;

private:
    void setProjection();
    void testPrint() const;

    int pixelW_;    //!< Width of viewport (in pixels)
    int pixelH_;    //!< Height of viewport (in pixels)
    float unitX_;   //!< Coordinate x of base point of viewport (in gl units)
    float unitY_;   //!< Coordinate y of base point of viewport (in gl units)
    float unitW_;   //!< Width of viewport (in gl units)
    float unitH_;   //!< Height of viewport (in gl units)
    float panX_;    //!< Pannig by coordinate x (in gl units)
    float panY_;    //!< Pannig by coordinate y (in gl units)

    float unitInPixel_;     //!< gl units in one pixel

    const float minUnitInPixel_;    //!< minimum value of unitInPixel_
    const float maxUnitInPixel_;    //!< maximum value of unitInPixel_
    const float zoomStep_;          //!< change of unitInPixel_ in one step

    const GLfloat zNear_;
    const GLfloat zFar_;
    const GLfloat zCamera_;
    glm::mat4 proj_;
};


}
