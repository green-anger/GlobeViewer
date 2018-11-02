#pragma once

#include <functional>
#include <memory>
#include <tuple>

#include <boost/signals2.hpp>

#include "LoadGL.h"


namespace gv {


class Projector;


/*!
 * \brief DataKeeper is a central storage for all OpenGL related variables.
 *
 * Every OpenGL variable is kept within DataKeeper. If any other module needs to read
 * or change some variables it must do so via public methods. Some calculation and
 * varialbe modifications may be performed by DataKeeper itself if it allows for speeding up.
 */
class DataKeeper
{
public:
    DataKeeper();
    ~DataKeeper();

    //! Initialize DataKeeper.
    void init();

    //! Rotate the Globe.
    void rotateGlobe( int pixelX, int pixelY );

    //! Rotate the Globe to initial state.
    void balanceGlobe();

    //! Move projection center to a point.
    void centerAt( int pixelX, int pixelY );

    //! Update map tiles texture with new data.
    void updateTexture( std::vector<GLfloat> /*vbo*/, int /*texW*/, int /*texH*/, std::vector<unsigned char> /*data*/ );

    //! Provide data on simple triangle for rendering.
    std::tuple<GLuint, GLsizei> simpleTriangle() const;

    //! Provide data on wire-frame model of the Globe for rendering.
    std::tuple<GLuint, GLsizei> wireGlobe() const;
    
    //! Provide data on map tiles for rendering.
    std::tuple<GLuint, GLuint, GLsizei> mapTiles() const;

    //! Request for a value of units in one meter.
    boost::signals2::signal<float()> getUnitInMeter;
    
    //! Request for a value of meters in one pixel.
    boost::signals2::signal<float()> getMeterInPixel;
    
    //! Request for a value of projected meters at a particular screen pixel.
    boost::signals2::signal<std::tuple<double, double>( int, int )> getMetersAtPixel;
    
    //! Request for pointer to Projector instance.
    boost::signals2::signal<std::shared_ptr<Projector>()> getProjector;
    
    //! Signal that the Globe has been rotated.
    boost::signals2::signal<void()> globeRotated;
    
    //! Signal that map is ready for rendering.
    boost::signals2::signal<void()> mapReady;

private:
    //! Calculate circles of latitude and meridians.
    void composeWireGlobe();

    float unitInMeter_;         //!< Non-changable value of units in one meter.

    std::shared_ptr<Projector> projector_;  //!< Pointer to Projector instance.

    GLuint vaoST_;              //!< Vertex array object for Simple Triangle.
    GLuint vboST_;              //!< Vertex buffer object for Simple Triangle.
    GLsizei numST_;             //!< Number of vertices for Simple Triangle.

    GLuint vaoWire_;            //!< Vertex array object for wire-frame model of the Globe.
    GLuint vboWire_;            //!< Vertex buffer object for wire-frame model of the Globe.
    GLsizei numWire_;           //!< Number of vertices for wire-frame model of the Globe.
    
    GLuint vaoMap_;             //!< Vertex array object for map tiles.
    GLuint vboMap_;             //!< Vertex buffer object for map tiles.
    GLuint texMap_;             //!< Texture for map tiles.
    GLsizei numMap_;            //!< Number of vertices for map tiles.

    double rotatedLon_;         //!< Current degree value the Globe rotated along longitude.
    double rotatedLat_;         //!< Current degree value the Globe rotated along latitude.
};


}
