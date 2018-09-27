#include <exception>
#include <iostream>
#include <thread>
#include <tuple>

//#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Renderer.h"
#include "Shader.h"


namespace gw
{


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

        glm::mat4 proj = glm::ortho( -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 100.0f );

        shaderSimple_->use();
        glUniformMatrix4fv( ssProj_, 1, GL_FALSE, glm::value_ptr( proj ) );
    }


}
