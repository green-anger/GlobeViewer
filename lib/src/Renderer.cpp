#include <exception>
#include <iostream>
#include <thread>
#include <tuple>

//#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DataKeeper.h"
#include "Renderer.h"
#include "Shader.h"
#include "Viewport.h"


namespace gv {


Renderer::Renderer( const std::shared_ptr<const DataKeeper>& dataKeeper, const std::shared_ptr<const Viewport>& viewport )
    : dataKeeper_( dataKeeper )
    , viewport_( viewport )
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

    const auto proj = viewport_->projection();

    shaderSimple_->use();
    glUniformMatrix4fv( ssProj_, 1, GL_FALSE, glm::value_ptr( proj ) );

    {   // Simple Triangle
        glUniform4fv( ssColor_, 1, glm::value_ptr( glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f ) ) );

        std::tuple<GLuint, std::size_t> params = dataKeeper_->simpleTriangle();
        const auto vao = std::get<0>( params );
        const auto num = std::get<1>( params );

        if ( num > 0 )
        {
            glBindVertexArray( vao );
            glDrawArrays( GL_TRIANGLES, 0, num );
        }
    }

    {   // Wire Globe
        glUniform4fv( ssColor_, 1, glm::value_ptr( glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f ) ) );

        std::tuple<GLuint, std::size_t> params = dataKeeper_->wireGlobe();
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
