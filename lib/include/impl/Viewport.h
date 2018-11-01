#pragma once

#include <map>
#include <tuple>

#include <boost/signals2.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include "LoadGL.h"
#include "type/ViewData.h"


namespace gv {


/*!
 * \brief Viewport controls dimensions and scale information of OpenGL viewport.
 *
 * Basically there are three different measurement units Viewport knows of:
 * meters, pixels and (abstract) OpenGL units. Meters are required for 
 * projection calculation of the globe, pixels - for positioning on the screen
 * and GL units - for correct representation on graphic driver. The most 
 * important thing is to keep correct ratio of these units, i.e. how many
 * meters in one pixel or how many pixels in one GL unit.
 *
 * There are three basic view operation that have impact on unit values:
 * panning, resizing and zooming view. Panning will change screen dimensions
 * in GL units, resizing - in pixels and zooming will change units ratio.
 */
class Viewport
{
public:
    Viewport();
    ~Viewport();

    //! Resize the viewport.
    void resize( int w, int h );

    //! Move the viewport.
    void move( int x, int y );
    
    //! Change scale.
    void zoom( int steps );
    
    //! Place point [0, 0] at the center of the viewport.
    void center();

    //! Provide information on the viewport current dimensions and scale.
    ViewData viewData() const;

    //! Provide current value of units in one meter.
    float unitInMeter() const;

    //! Provide current value of meters in one  pixel.
    float meterInPixel() const;

    //! Provide OpenGL projection.
    glm::mat4 projection() const;

    //! Provide a value of projected meters at a particular screen pixel.
    std::tuple<double, double> metersAtPixel( int x, int y ) const;

    //! Signal that the viewport has changed.
    boost::signals2::signal<void( ViewData )> viewUpdated;

private:
    //! Recalculate OpenGL projection with current dimensions and scale.
    void setProjection();

    //! Calculate current map zoom level for a given map tile width.
    int mapZoomLevel( int tileWidth ) const;

    int pixelW_;    //!< Width of viewport (in pixels).
    int pixelH_;    //!< Height of viewport (in pixels).
    float unitX_;   //!< Coordinate x of base point of viewport (in GL units).
    float unitY_;   //!< Coordinate y of base point of viewport (in GL units).
    float unitW_;   //!< Width of viewport (in GL units).
    float unitH_;   //!< Height of viewport (in GL units).
    float panX_;    //!< Panning by coordinate x (in GL units).
    float panY_;    //!< Panning by coordinate y (in GL units).

    static const float unitInMeter_;                //!< GL units in one meter.
    float unitInPixel_;                             //!< GL units in one pixel.
    float meterInPixel_;                            //!< Meters in one pixel.

    static const float minLen_;                     //!< Minimum length to display in 1920 pixels.
    static const float maxLen_;                     //!< Maximum Globe diameter plus some space to fit whole Globe.
    static const float minUnitInPixel_;             //!< Minimum value of unitInPixel_.
    static const float maxUnitInPixel_;             //!< Maximum value of unitInPixel_.
    static const std::map<float, float> zoomMap_;   //!< Change of unitInPixel_ in one step depending on current zoom.

    const GLfloat zNear_;                           //!< The nearest plane of frustum.
    const GLfloat zFar_;                            //!< The farthest plane of frustum.
    const GLfloat zCamera_;                         //!< Camera position on axis z.
    glm::mat4 proj_;                                //!< OpenGL projection.
};


}
