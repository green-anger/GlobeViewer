#pragma once

#include <memory>
#include <tuple>

#include "LoadGL.h"


namespace gv {


class Projector;


class DataKeeper
{
public:
    DataKeeper();
    ~DataKeeper();

    std::tuple<GLuint, std::size_t> simpleTriangle() const;

private:
    std::shared_ptr<Projector> projector_;

    GLuint vaoST_;  //!< vao for Simple Triangle
    GLuint vboST_;  //!< vbo for Simple Triangle
    GLuint numST_;  //!< number of vertices for Simple Triangle
};


}
