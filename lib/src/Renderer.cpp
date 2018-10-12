#include <exception>
#include <iostream>
#include <thread>
#include <tuple>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Renderer.h"
#include "Shader.h"


namespace gv {


Renderer::Renderer()
{
    shaderSimple_.reset( new support::Shader( "shaders/simple.vs", "shaders/simple.fs" ) );

    if ( !shaderSimple_->isValid() )
        throw std::logic_error( "Shader initialization failed!" );

    ssProj_ = shaderSimple_->uniformLocation( "proj" );
    ssColor_ = shaderSimple_->uniformLocation( "colorIn" );
}


Renderer::~Renderer()
{
}


void Renderer::render()
{
    glClearColor( 0.2f, 0.3f, 0.3f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    boost::optional<glm::mat4> optProjection = getProjection();

    if ( !optProjection )
    {
        throw std::logic_error( "Renderer cannot get projection!" );
    }

    const auto proj = *optProjection;

    shaderSimple_->use();
    glUniformMatrix4fv( ssProj_, 1, GL_FALSE, glm::value_ptr( proj ) );

    {   // Simple Triangle
        glUniform4fv( ssColor_, 1, glm::value_ptr( glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f ) ) );

        boost::optional<std::tuple<GLuint, std::size_t>> optSimpleTriangle = renderSimpleTriangle();

        if ( optSimpleTriangle )
        {
            std::tuple<GLuint, std::size_t> params = *optSimpleTriangle;
            const auto vao = std::get<0>( params );
            const auto num = std::get<1>( params );

            if ( num > 0 )
            {
                glBindVertexArray( vao );
                glDrawArrays( GL_TRIANGLES, 0, num );
            }
        }
    }

    {   // Wire Globe
        glUniform4fv( ssColor_, 1, glm::value_ptr( glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f ) ) );

        boost::optional<std::tuple<GLuint, std::size_t>> optWireGlobe = renderWireGlobe();

        if ( optWireGlobe )
        {
            std::tuple<GLuint, std::size_t> params = *optWireGlobe;
            const auto vao = std::get<0>( params );
            const auto num = std::get<1>( params );

            if ( num > 0 )
            {
                glLineWidth( 1.0f );
                glBindVertexArray( vao );
                glDrawArrays( GL_LINES, 0, num );
            }
        }
    }
}


}
