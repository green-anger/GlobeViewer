#pragma once

#include <tuple>

#include "LoadGL.h"


namespace gw {


class DataKeeper
{
public:
    DataKeeper();
    ~DataKeeper();

    std::tuple<GLuint, std::size_t> simpleTriangle() const;

private:
    GLuint vaoST_;  //!< vao for Simple Triangle
    GLuint vboST_;  //!< vbo for Simple Triangle
    GLuint numST_;  //!< number of vertices for Simple Triangle
};


}
