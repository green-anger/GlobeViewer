#pragma once

#include <memory>
#include <tuple>

#include <boost/signals2.hpp>

#include <glm/ext/matrix_float4x4.hpp>

#include "LoadGl.h"


namespace gv {


namespace support {
    class Shader;
}


class Renderer
{
public:
    explicit Renderer();
    ~Renderer();

    void render();

    boost::signals2::signal<glm::mat4()> getProjection;
    boost::signals2::signal<std::tuple<GLuint, std::size_t>()> renderSimpleTriangle;
    boost::signals2::signal<std::tuple<GLuint, std::size_t>()> renderWireGlobe;

private:
    std::unique_ptr<support::Shader> shaderSimple_;
    GLint ssProj_;
    GLint ssColor_;
};


}
