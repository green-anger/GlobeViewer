#include <iostream>
#include <memory>
#include <string>
#include <tuple>

#include <GLFW/glfw3.h>

#include "support/FpsCounter.h"
#include "GlobeViewer.h"


void windowFocusCallback( GLFWwindow* window, int focused );
void framebufferSizeCallback( GLFWwindow* window, int width, int height );
void mouseCallback( GLFWwindow* window, double xpos, double ypos );
void mouseButtonCallback( GLFWwindow* window, int button, int action, int mods );
void keyCallback( GLFWwindow* window, int key, int scancode, int action, int mode );

std::tuple<GLFWwindow*, int, int> setupWindow( const std::string& title );

std::unique_ptr<gv::GlobeViewer> globalViewer;
bool drag = false;
bool firstClick = true;
double lastX;
double lastY;


int main( int argc, char** argv )
{
    std::tuple<GLFWwindow*, int, int> tup = setupWindow( "GlobeViewer" );
    auto window = std::get<0>( tup );

    if ( !window )
        return -1;

    globalViewer.reset( new gv::GlobeViewer() );

    if ( !globalViewer->validSetup() )
        return -1;

    const auto width = std::get<1>( tup );
    const auto height = std::get<2>( tup );
    globalViewer->resize( width, height );

    glfwSetWindowFocusCallback( window, windowFocusCallback );
    glfwSetFramebufferSizeCallback( window, framebufferSizeCallback );
    glfwSetCursorPosCallback( window, mouseCallback );
    glfwSetMouseButtonCallback( window, mouseButtonCallback );
    glfwSetKeyCallback( window, keyCallback );

    gv::FpsCounter fpsC;

    while ( !glfwWindowShouldClose( window ) )
    {
        auto fps = fpsC.addFrame();

        if ( fps > 0 )
        {
            std::cout << "FPS: " << fps << std::endl;
        }

        glfwPollEvents();
        globalViewer->render();
        glfwSwapBuffers( window );
    }

    glfwDestroyWindow( window );
    glfwTerminate();

    return 0;
}


void windowFocusCallback( GLFWwindow* window, int focused )
{
    if ( focused > 0 )
    {
        firstClick = true;
    }
}


void framebufferSizeCallback( GLFWwindow* window, int width, int height )
{
    globalViewer->resize( width, height );
}


void mouseCallback( GLFWwindow* window, double xpos, double ypos )
{
    if ( firstClick )
    {
        lastX = xpos;
        lastY = ypos;
        firstClick = false;
    }

    double xoff = xpos - lastX;
    double yoff = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    if ( drag )
    {
        globalViewer->move( static_cast<int>( xoff ), static_cast<int>( yoff ) );
    }
}


void mouseButtonCallback( GLFWwindow* window, int button, int action, int mods )
{
    if ( GLFW_MOUSE_BUTTON_1 == button && GLFW_PRESS == action )
    {
        drag = true;
    }
    else if ( GLFW_MOUSE_BUTTON_1 == button && GLFW_RELEASE == action )
    {
        drag = false;
    }
}


void keyCallback( GLFWwindow* window, int key, int scancode, int action, int mode )
{
    if ( GLFW_KEY_ESCAPE == key && GLFW_PRESS == action )
    {
        glfwSetWindowShouldClose( window, GLFW_TRUE );
    }

    if ( !drag )
    {
        int xoff = 0;
        int yoff = 0;

        const int mod = GLFW_MOD_SHIFT == mode ? 10 : 1;
        xoff += mod * ( glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS ? 1 : 0 );
        xoff -= mod * ( glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS ? 1 : 0 );
        yoff += mod * ( glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS ? 1 : 0 );
        yoff -= mod * ( glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS ? 1 : 0 );

        if ( xoff != 0 || yoff != 0 )
        {
            globalViewer->move( xoff, yoff );
        }
    }
}


std::tuple<GLFWwindow*, int, int> setupWindow( const std::string& title )
{
    if ( !glfwInit() )
    {
        return std::make_tuple( nullptr, 0, 0 );
    }

    auto monitor = glfwGetPrimaryMonitor();
    auto mode = glfwGetVideoMode( monitor );

    float ratio = 0.8f;
    const auto width = static_cast< int >( ratio * mode->width );
    const auto height = static_cast< int >( ratio * mode->height );

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_SAMPLES, 8 );

    GLFWwindow* window = glfwCreateWindow( width, height, title.c_str(), 0, 0 );
    if ( !window )
    {
        return std::make_tuple( nullptr, 0 , 0 );
    }

    float revRat = ( 1.0f - ratio ) / 2;
    int xpos = static_cast<int>( revRat * mode->width );
    int ypos = static_cast<int>( revRat * mode->height );
    glfwSetWindowPos( window, xpos, ypos );
    glfwMakeContextCurrent( window );
    glfwSwapInterval( 1 );

    return std::make_tuple( window, width, height );
}
