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


/*!
 * \brief Draws OpenGL entities that were marked for render.
 *
 * It simply requests all objects from DataKeeper and invokes couple of
 * OpenGL commands to draw as fast as possible. It doesn't do much, but
 * has a beneficial effect of keeping all relevant operations in one place,
 * so it's easier to maintain and control display of different parts of
 * the final image scene.
 */
class Renderer
{
public:
    explicit Renderer();
    ~Renderer();

    //! Render the view.
    void render();

    //! Mark map as ready or not ready for rendering.
    void setMapReady( bool );

    //! Turn rendering of wire-frame model of the Globe on or off.
    void setDrawWires( bool );

    //! Turn rendering of map on or off.
    void setDrawMap( bool );

    //! Request OpenGL projection.
    boost::signals2::signal<glm::mat4()> getProjection;

    //! Request rendering data for simple triangle.
    boost::signals2::signal<std::tuple<GLuint, GLsizei>()> renderSimpleTriangle;

    //! Request rendering data for wire-frame model of the Globe.
    boost::signals2::signal<std::tuple<GLuint, GLsizei>()> renderWireGlobe;

    //! Request rendering data for map.
    boost::signals2::signal<std::tuple<GLuint, GLuint, GLsizei>()> renderMapTiles;

private:
    std::unique_ptr<support::Shader> shaderSimple_;     //!< Simple shader.
    GLint ssProj_;                                      //!< Location of projection uniform in simple shader.
    GLint ssColor_;                                     //!< Location of colour uniform in simple shader.
    std::unique_ptr<support::Shader> shaderTexture_;    //!< Texture shader.
    GLint stProj_;                                      //!< Location of projection uniform in texture shader.
    GLint stSample_;                                    //!< Location of sample uniform in texture shader.
    bool mapReady_;                                     //!< Indicator of map readiness for rendering.
    bool drawWires_;                                    //!< Indicator of requirement to draw wire-frame globe.
    bool drawMap_;                                      //!< Indicator of requirement to draw map.
};


}
