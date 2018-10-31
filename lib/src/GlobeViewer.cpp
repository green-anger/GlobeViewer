#include <atomic>
#include <fstream>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>
#include <thread>

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/signals2.hpp>

#include "DataKeeper.h"
#include "Defines.h"
#include "GlobeViewer.h"
#include "MapGenerator.h"
#include "Profiler.h"
#include "Projector.h"
#include "Renderer.h"
#include "TileManager.h"
#include "Viewport.h"


using namespace boost::asio;


/*!
 * \brief Contains all classes and functions related to GlobeViewer
 */
namespace gv
{


/*!
 * \brief Implementation of GlobeViewer.
 * 
 * The main trick is to make all OpenGL calls in one thread, otherwise the application
 * will not work properly or may even crash. To make it possible instance of
 * boost::asio::io_context (ioc) is used with a single thread. This does the trick and
 * additionally eliminates the need of synchronization between different GlobeViewer calls.
 * Because every GlobeViewer API call just post some task to ioc object and every 
 * such task will be undertaken consecutively and in the same thread (implicit strand
 * in terms of Boost.Asio).
 */
struct GlobeViewer::Impl
{
public:
    Impl( std::function<void()> );
    ~Impl();

    //! Initialize OpenGL.
    bool initGl() const;

    //! Initialize data.
    void initData();

    std::atomic<bool> valid;                    //!< Indicator of validity of Impl initialization.

    std::function<void()> makeCurrent;          //!< Callable that makes some OpenGL context current.

    std::shared_ptr<DataKeeper> dataKeeper;     //!< Keeps all OpenGL relative variables.
    std::shared_ptr<MapGenerator> mapGenerator; //!< Generates a single texture from map tiles.
    std::shared_ptr<Projector> projector;       //!< Projects the globe to plane.
    std::shared_ptr<Renderer> renderer;         //!< Renders all OpenGL entities.
    std::shared_ptr<TileManager> tileManager;   //!< Downloads and caches map tiles from tile servers.
    std::shared_ptr<Viewport> viewport;         //!< Calculates dimensions of the view.

    boost::asio::io_context ioc;                //!< Allows implementing task queue.
    executor_work_guard<io_context::executor_type> work;    //!< Provides work to ioc and doesn't let it stop.
    std::vector<std::thread> threads;           //!< Vector of worker thread. To make it all work properly strictly one is needed.
};


/*!
* Constructor's parameters are redirected from GlobeViewer ctor.
* \param[in] func A callable capable of making current some OpenGL context.
*/
GlobeViewer::Impl::Impl( std::function<void()> func )
    : valid( false )
    , makeCurrent( func )
    , ioc()
    , work( make_work_guard( ioc ) )
{
    threads.emplace_back( [this]() { ioc.run(); } );
}


GlobeViewer::Impl::~Impl()
{
    work.reset();
    ioc.stop();

    for ( auto&& t : threads )
    {
        if ( t.joinable() )
        {
            t.join();
        }
    }

    std::ofstream file( "profile_map.txt", std::ios::out );
    support::Profiler::printStatistics( file );
    file.close();
}


/*!
 * Cannot be called in constructor because of OpenGL calls limit -
 * they must be in the same thread. GlobeViewer constructor posts initGl() to ioc.
 * \return True - OpenGL successfully initialized, false - otherwise.
 */
bool GlobeViewer::Impl::initGl() const
{
    makeCurrent();
    return gladLoadGL();
}


/*!
 * Every member must be initialized before proceeding. However it cannot be done
 * in constructor because of OpenGL calls limit - they must be in the same thread.
 * And there is no way to force user to create GlobeViewer instance in a particular
 * thread, hence GlobeViewer constructor posts initData() to ioc.
 */
void GlobeViewer::Impl::initData()
{
    dataKeeper.reset( new DataKeeper() );
    mapGenerator.reset( new MapGenerator() );
    projector.reset( new Projector() );
    renderer.reset( new Renderer() );
    tileManager.reset( new TileManager() );
    viewport.reset( new Viewport() );

    namespace ph = std::placeholders;

    dataKeeper->getUnitInMeter.connect( std::bind( &Viewport::unitInMeter, viewport ) );
    dataKeeper->getMeterInPixel.connect( std::bind( &Viewport::meterInPixel, viewport ) );
    dataKeeper->getMetersAtPixel.connect( std::bind( &Viewport::metersAtPixel, viewport, ph::_1, ph::_2 ) );
    dataKeeper->getProjector.connect( [this]() -> auto { return projector; } );
    dataKeeper->globeRotated.connect( std::bind( &MapGenerator::updateGlobe, mapGenerator ) );
    dataKeeper->mapReady.connect( std::bind( &Renderer::setMapReady, renderer, ph::_1 ) );

    viewport->viewUpdated.connect( std::bind( &MapGenerator::updateViewData, mapGenerator, ph::_1 ) );

    renderer->getProjection.connect( std::bind( &Viewport::projection, viewport ) );
    renderer->renderSimpleTriangle.connect( std::bind( &DataKeeper::simpleTriangle, dataKeeper ) );
    renderer->renderWireGlobe.connect( std::bind( &DataKeeper::wireGlobe, dataKeeper ) );
    renderer->renderMapTiles.connect( std::bind( &DataKeeper::mapTiles, dataKeeper ) );

    mapGenerator->getProjector.connect( [this]() -> auto { return projector; } );
    mapGenerator->requestTiles.connect( std::bind( &TileManager::requestTiles, tileManager, ph::_1, ph::_2 ) );
    mapGenerator->mapReady.connect( std::bind( &Renderer::setMapReady, renderer, ph::_1 ) );
    mapGenerator->updateMapTexture.connect( [this]( std::vector<GLfloat> vbo, int w, int h, std::vector<unsigned char> data )
    {
        ioc.post( [vbo, w, h, data, this]
        {
            dataKeeper->updateTexture( vbo, w, h, data );
        } );
    } );

    tileManager->sendTiles.connect( std::bind( &MapGenerator::getTiles, mapGenerator, ph::_1 ) );

    dataKeeper->init();
    mapGenerator->init( viewport->viewData() );

    valid = true;
}


/*!
 * Constructor must receive means to make current some OpenGL context.
 * \param[in] func A callable capable of making current some OpenGL context.
 */
GlobeViewer::GlobeViewer( std::function<void()> func )
    : impl_( new GlobeViewer::Impl( func ) )
{
    std::promise<void> promInit;
    impl_->ioc.post( [&promInit, this]
    {
        if ( !impl_->initGl() )
        {
            throw std::logic_error( "OpenGL initialization failed!" );
        }

        impl_->initData();
        promInit.set_value();
    } );
    promInit.get_future().wait();
}


GlobeViewer::~GlobeViewer()
{
}


/*!
 * Calling any other API method before getting true from this one may result in undefined behavior.
 * \return true if setup is valid, false otherwise
 */
bool GlobeViewer::validSetup() const
{
    return impl_ ? impl_->valid.load() : false;
}


/*!
 * It must be called every time the view must be renewed.
 * It's often done by timer to get constant frame rate.
 */
void GlobeViewer::render()
{
    impl_->ioc.post( [this] {
        if ( impl_ )
        {
            impl_->makeCurrent();
            impl_->renderer->render();
        }
    } );
}


/*!
 * Change the view dimensions. Usually it's called every time the owner window resizes
 * and the view needs to fill whole client area.
 * \param[in] w New width of the view.
 * \param[in] h New height of the view.
 */
void GlobeViewer::resize( int w, int h )
{
    impl_->ioc.post( [this, w, h] {
        if ( impl_ )
        {
            impl_->viewport->resize( w, h );
        }
    } );
}


/*!
 * Panning view around {x, y} axises.
 * \param[in] x Offset along x axis. Positive - move to the right, negative - move to the left. 
 * \param[in] y Offset along y axis. Positive - move up, negative - move down. 
 */
void GlobeViewer::move( int x, int y )
{
    impl_->ioc.post( [this, x, y] {
        if ( impl_ )
        {
            impl_->viewport->move( x, y );
        }
    } );
}


/*!
 * \param[in] steps Number of steps to zoom. Positive - zooming in, negative - zooming out.
 */
void GlobeViewer::zoom( int steps )
{
    impl_->ioc.post( [this, steps] {
        if ( impl_ )
        {
            impl_->viewport->zoom( steps );
        }
    } );
}


/*!
 * Useful when inaccurate view panning moves the Globe off the screen.
 */
void GlobeViewer::centerView()
{
    impl_->ioc.post( [this] {
        if ( impl_ )
        {
            impl_->viewport->center();
        }
    } );
}


/*!
 * Projection center is set to [0, 0]. That means a vector going from the Globe center to [0, 0]
 * is directed at the viewer and the one going to the North Pole is directed up.
 */
void GlobeViewer::baseState()
{
    impl_->ioc.post( [this] {
        if ( impl_ )
        {
            impl_->dataKeeper->balanceGlobe();
        }
    } );
}


/*!
 * Rotation degree in each direction is based on view pixel offset (set in this method)
 * and current Globe scale (changed in zoom method).
 * \param[in] x Positive - rotating right, negative - rotating left.
 * \param[in] y Positive - rotating up, negative - rotating down.
 *
 * Right means that if we're looking at the North Pole from above the Globe is rotating counterclockwise.
 * Moving up or down to the extremes will never turn the Globe upside down
 * but will fix it at the latitude limit (-90 or +90 degree).
 */
void GlobeViewer::rotate( int x, int y )
{
    impl_->ioc.post( [this, x, y] {
        if ( impl_ )
        {
            impl_->dataKeeper->rotateGlobe( x, y );
        }
    } );
}


/*!
 * Must point at a view pixel. If that pixel is inside the Globe projection,
 * the Globe will rotate so that new projection center is at the new point.
 * Otherwise the point is ignored.
 * \param[in] x Coordinate x of the pixel.
 * \param[in] y Coordinate y of the pixel.
 */
void GlobeViewer::projectCenterAt( int x, int y )
{
    impl_->ioc.post( [this, x, y] {
        if ( impl_ )
        {
            impl_->dataKeeper->centerAt( x, y );
        }
    } );
}


/*!
 * Must choose one of the values from TileServer enumerator.
 * \param[in] ts TileServer value.
 * \exception std::logic_error In case of unknown value. Throw in a different thread.
 */
void GlobeViewer::setTileSource( TileServer ts )
{
    impl_->ioc.post( [this, ts] {
        if ( impl_ )
        {
            impl_->mapGenerator->updateTileServer( ts );
        }
    } );
}


/*!
 * \param[in] val True - turn on, false - turn off.
 */
void GlobeViewer::setWireFrameView( bool val )
{
    impl_->ioc.post( [this, val] {
        if ( impl_ )
        {
            impl_->renderer->setDrawWires( val );
        }
    } );
}


/*!
* \param[in] val True - turn on, false - turn off.
*/
void GlobeViewer::setMapTilesView( bool val )
{
    impl_->ioc.post( [this, val] {
        if ( impl_ )
        {
            impl_->renderer->setDrawMap( val );
        }
    } );
}


/*!
 * \warning Call this at the end of the main function if an instance of
 * GlobeViewer is a global variable. Otherwise it will conflict with
 * destruction of static variables used in GlobeViewer and may crash!
 */
void GlobeViewer::cleanup()
{
    impl_.reset();
}


}
