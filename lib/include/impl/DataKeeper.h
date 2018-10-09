#pragma once

#include <functional>
#include <memory>
#include <tuple>

#include <boost/signals2.hpp>

#include "LoadGL.h"


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

    boost::signals2::signal<float()> getUnitInMeter;
    boost::signals2::signal<float()> getMeterInPixel;

    std::tuple<GLuint, std::size_t> simpleTriangle() const;
    std::tuple<GLuint, std::size_t> wireGlobe() const;

private:
    void composeWireGlobe();

    float unitInMeter_;

    std::shared_ptr<Projector> projector_;

    GLuint vaoST_;  //!< vao for Simple Triangle
    GLuint vboST_;  //!< vbo for Simple Triangle
    GLuint numST_;  //!< number of vertices for Simple Triangle

    GLuint vaoWire_;    //!< vao for wire-frame model of the Globe
    GLuint vboWire_;    //!< vbo for wire-frame model of the Globe
    GLuint numWire_;    //!< number of vertices for wire-frame model of the Globe

    double rotatedLon_;
    double rotatedLat_;
};


}
