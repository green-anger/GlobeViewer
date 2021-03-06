#include <functional>
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
void scrollCallback( GLFWwindow* window, double xpos, double ypos );
void keyCallback( GLFWwindow* window, int key, int scancode, int action, int mode );

std::tuple<GLFWwindow*, int, int> setupWindow( const std::string& title );

std::unique_ptr<gv::GlobeViewer> globeViewer;
bool drag = false;
bool shift = false;
bool fullscreen = false;
bool firstClick = true;
double lastX;
double lastY;
int lastPosX;
int lastPosY;
int lastWidth;
int lastHeight;
gv::TileServer tileServer = gv::TileServer::OSM;
bool drawWireFrameView = true;
bool drawMapTilesView = true;


int main( int argc, char** argv )
{
    std::tuple<GLFWwindow*, int, int> tup = setupWindow( "GlobeViewer" );
    auto window = std::get<0>( tup );

    if ( !window )
        return -1;

    glfwMakeContextCurrent( nullptr );

    globeViewer.reset( new gv::GlobeViewer( std::bind( glfwMakeContextCurrent, window ) ) );

    if ( !globeViewer->validSetup() )
        return -1;

    const auto width = std::get<1>( tup );
    const auto height = std::get<2>( tup );
    globeViewer->resize( width, height );

    glfwSetWindowFocusCallback( window, windowFocusCallback );
    glfwSetFramebufferSizeCallback( window, framebufferSizeCallback );
    glfwSetCursorPosCallback( window, mouseCallback );
    glfwSetMouseButtonCallback( window, mouseButtonCallback );
    glfwSetScrollCallback( window, scrollCallback );
    glfwSetKeyCallback( window, keyCallback );

    gv::FpsCounter fpsC;

    while ( !glfwWindowShouldClose( window ) )
    {
        //auto fps = fpsC.addFrame();

        //if ( fps > 0 )
        //{
        //    std::cout << "FPS: " << fps << std::endl;
        //}

        glfwPollEvents();
        globeViewer->render();
        glfwSwapBuffers( window );
    }

    glfwDestroyWindow( window );
    glfwTerminate();

    globeViewer->cleanup();

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
    globeViewer->resize( width, height );
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
        if ( shift )
        {
            globeViewer->rotate( static_cast< int >( xoff ), static_cast< int >( yoff ) );
        }
        else
        {
            globeViewer->move( static_cast< int >( xoff ), static_cast< int >( yoff ) );
        }
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


void scrollCallback( GLFWwindow* window, double xpos, double ypos )
{
    globeViewer->zoom( static_cast<int>( ypos ) );
}


void keyCallback( GLFWwindow* window, int key, int scancode, int action, int mode )
{
    if ( GLFW_KEY_ESCAPE == key && GLFW_PRESS == action )
    {
        glfwSetWindowShouldClose( window, GLFW_TRUE );
    }
    else if ( GLFW_KEY_C == key && GLFW_PRESS == action )
    {
        globeViewer->centerView();
    }
    else if ( GLFW_KEY_B == key && GLFW_PRESS == action )
    {
        globeViewer->baseState();
    }
    else if ( GLFW_KEY_P == key && GLFW_PRESS == action )
    {
        globeViewer->projectCenterAt( lastX, lastY );
    }
    else if ( GLFW_KEY_T == key && GLFW_PRESS == action )
    {
        if ( gv::TileServer::OSM == tileServer )
        {
            tileServer = gv::TileServer::GIS;
        }
        else
        {
            tileServer = gv::TileServer::OSM;
        }

        globeViewer->setTileSource( tileServer );
    }
    else if ( GLFW_KEY_1 == key && GLFW_PRESS == action )
    {
        drawWireFrameView = !drawWireFrameView;

        globeViewer->setWireFrameView( drawWireFrameView );
    }
    else if ( GLFW_KEY_2 == key && GLFW_PRESS == action )
    {
        drawMapTilesView = !drawMapTilesView;

        globeViewer->setMapTilesView( drawMapTilesView );
    }
    else if ( GLFW_KEY_F11 == key && GLFW_PRESS == action )
    {
        if ( !fullscreen )
        {
            glfwGetWindowSize( window, &lastWidth, &lastHeight );
            glfwGetWindowPos( window, &lastPosX, &lastPosY );
            auto monitor = glfwGetPrimaryMonitor();
            auto mode = glfwGetVideoMode( monitor );
            glfwSetWindowMonitor( window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate );
        }
        else
        {
            glfwSetWindowMonitor( window, nullptr, lastPosX, lastPosY, lastWidth, lastHeight, GLFW_DONT_CARE );
        }

        fullscreen = !fullscreen;
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
            globeViewer->move( xoff, yoff );
        }
    }

    shift = ( GLFW_MOD_SHIFT == mode );
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
