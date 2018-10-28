#include <exception>
#include <thread>
#include <tuple>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Renderer.h"
#include "Shader.h"


namespace gv {


Renderer::Renderer()
    : mapReady_( false )
    , drawWires_( true )
    , drawMap_( true )
{
    shaderSimple_.reset( new support::Shader( "shaders/simple.vs", "shaders/simple.fs" ) );
    shaderTexture_.reset( new support::Shader( "shaders/texture.vs", "shaders/texture.fs" ) );

    if ( !shaderSimple_->isValid() || !shaderTexture_->isValid() )
    {
        throw std::logic_error( "Shader initialization failed!" );
    }

    ssProj_ = shaderSimple_->uniformLocation( "proj" );
    ssColor_ = shaderSimple_->uniformLocation( "colorIn" );
    stProj_ = shaderTexture_->uniformLocation( "proj" );
    stSample_ = shaderTexture_->uniformLocation( "sample" );
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

    shaderTexture_->use();
    glUniformMatrix4fv( stProj_, 1, GL_FALSE, glm::value_ptr( proj ) );

    if ( drawMap_ && mapReady_ )
    {   // Map tiles
        boost::optional<std::tuple<GLuint, GLuint, GLsizei>> optMapTiles = renderMapTiles();

        if ( optMapTiles )
        {
            std::tuple<GLuint, GLuint, GLsizei> params = *optMapTiles;
            const auto vao = std::get<0>( params );
            const auto tex = std::get<1>( params );
            const auto num = std::get<2>( params );

            if ( num > 0 )
            {
                glBindVertexArray( vao );
                glActiveTexture( GL_TEXTURE0 );
                glBindTexture( GL_TEXTURE_2D, tex );
                glDrawArrays( GL_TRIANGLES, 0, num );
                glBindTexture( GL_TEXTURE_2D, 0 );
                glBindVertexArray( 0 );
            }
        }
    }

    shaderSimple_->use();
    glUniformMatrix4fv( ssProj_, 1, GL_FALSE, glm::value_ptr( proj ) );

    /*
    {   // Simple Triangle
        glUniform4fv( ssColor_, 1, glm::value_ptr( glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f ) ) );

        boost::optional<std::tuple<GLuint, GLsizei>> optSimpleTriangle = renderSimpleTriangle();

        if ( optSimpleTriangle )
        {
            std::tuple<GLuint, std::size_t> params = *optSimpleTriangle;
            const auto vao = std::get<0>( params );
            const auto num = std::get<1>( params );

            if ( num > 0 )
            {
                glBindVertexArray( vao );
                glDrawArrays( GL_TRIANGLES, 0, num );
                glBindVertexArray( 0 );
            }
        }
    }
    //*/

    if ( drawWires_ )
    {   // Wire Globe
        glUniform4fv( ssColor_, 1, glm::value_ptr( glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f ) ) );

        boost::optional<std::tuple<GLuint, GLsizei>> optWireGlobe = renderWireGlobe();

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
                glBindVertexArray( 0 );
            }
        }
    }
}


void Renderer::setMapReady( bool val )
{
    mapReady_ = val;
}


void Renderer::setDrawWires( bool val )
{
    drawWires_ = val;
}


void Renderer::setDrawMap( bool val )
{
    drawMap_ = val;
}


}
