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
    void setMapReady( bool );
    void setDrawWires( bool );

    boost::signals2::signal<glm::mat4()> getProjection;
    boost::signals2::signal<std::tuple<GLuint, GLsizei>()> renderSimpleTriangle;
    boost::signals2::signal<std::tuple<GLuint, GLsizei>()> renderWireGlobe;
    boost::signals2::signal<std::tuple<GLuint, GLuint, GLsizei>()> renderMapTiles;

private:
    std::unique_ptr<support::Shader> shaderSimple_;
    GLint ssProj_;
    GLint ssColor_;
    std::unique_ptr<support::Shader> shaderTexture_;
    GLint stProj_;
    GLint stSample_;
    bool mapReady_;
    bool drawWires_;
};


}
