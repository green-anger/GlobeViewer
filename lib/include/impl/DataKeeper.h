#pragma once

#include <functional>
#include <memory>
#include <tuple>

#include <boost/signals2.hpp>

#include "LoadGL.h"
#include "type/TileMap.h"
#include "type/TileTexture.h"


namespace gv {


class Projector;


class DataKeeper
{
public:
    DataKeeper();
    ~DataKeeper();

    void init();
    void rotateGlobe( int pixelX, int pixelY );
    void balanceGlobe();

    void updateTexture( std::vector<GLfloat> /*vbo*/, int /*texW*/, int /*texH*/, std::vector<unsigned char> /*data*/ );

    std::tuple<GLuint, GLsizei> simpleTriangle() const;
    std::tuple<GLuint, GLsizei> wireGlobe() const;
    std::tuple<GLuint, GLuint, GLsizei> mapTiles() const;

    boost::signals2::signal<float()> getUnitInMeter;
    boost::signals2::signal<float()> getMeterInPixel;
    boost::signals2::signal<std::shared_ptr<Projector>()> getProjector;
    boost::signals2::signal<void()> globeRotated;
    boost::signals2::signal<void( bool )> mapReady;

private:
    void composeWireGlobe();

    float unitInMeter_;

    std::shared_ptr<Projector> projector_;

    GLuint vaoST_;      //!< vao for Simple Triangle
    GLuint vboST_;      //!< vbo for Simple Triangle
    GLsizei numST_;     //!< number of vertices for Simple Triangle

    GLuint vaoWire_;    //!< vao for wire-frame model of the Globe
    GLuint vboWire_;    //!< vbo for wire-frame model of the Globe
    GLsizei numWire_;   //!< number of vertices for wire-frame model of the Globe
    
    GLuint vaoMap_;     //!< vao for map tiles
    GLuint vboMap_;     //!< vbo for map tiles
    GLuint texMap_;     //!< texture for map tiles
    GLsizei numMap_;    //!< number of vertices for map tiles
    TileTexture tileTexture_;

    double rotatedLon_;
    double rotatedLat_;
};


}
