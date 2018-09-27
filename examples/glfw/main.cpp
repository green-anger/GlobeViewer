#include <iostream>
#include <memory>
#include <string>

#include <GLFW/glfw3.h>

#include "GlobeViewer.h"


GLFWwindow* setupWindow( const std::string& title );

std::unique_ptr<gw::GlobeViewer> globalViewer;


int main( int argc, char** argv )
{
    auto window = setupWindow( "GlobeViewer" );

    if ( !window )
        return -1;

    globalViewer.reset( new gw::GlobeViewer() );

    if ( !globalViewer->validSetup() )
        return -1;

    while ( !glfwWindowShouldClose( window ) )
    {
        glfwPollEvents();
        globalViewer->render();
        glfwSwapBuffers( window );
    }

    glfwDestroyWindow( window );
    glfwTerminate();

    return 0;
}


GLFWwindow* setupWindow( const std::string& title )
{
    if ( !glfwInit() )
        return nullptr;

    auto monitor = glfwGetPrimaryMonitor();
    auto mode = glfwGetVideoMode( monitor );

    std::cout << mode->width << ":" << mode->height << std::endl;

    float ratio = 0.8f;
    const auto width = ratio * mode->width;
    const auto height = ratio * mode->height;

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_SAMPLES, 8 );

    GLFWwindow* window = glfwCreateWindow( width, height, title.c_str(), 0, 0 );
    if ( !window )
        return 0;

    float revRat = ( 1.0f - ratio ) / 2;
    glfwSetWindowPos( window, revRat * mode->width, revRat * mode->height );
    glfwMakeContextCurrent( window );
    glfwSwapInterval( 1 );

    return window;
}
