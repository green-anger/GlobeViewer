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


namespace gv
{


struct GlobeViewer::Impl
{
public:
    Impl( std::function<void()> );
    ~Impl();

    bool initGl() const;
    void initData();

    bool valid;

    std::function<void()> makeCurrent;

    std::shared_ptr<DataKeeper> dataKeeper;
    std::shared_ptr<MapGenerator> mapGenerator;
    std::shared_ptr<Projector> projector;
    std::shared_ptr<Renderer> renderer;
    std::shared_ptr<TileManager> tileManager;
    std::shared_ptr<Viewport> viewport;

    boost::asio::io_context ioc;
    executor_work_guard<io_context::executor_type> work;
    std::vector<std::thread> threads;
};


GlobeViewer::Impl::Impl( std::function<void()> func )
    : valid( false )
    , makeCurrent( func )
    , ioc()
    , work( make_work_guard( ioc ) )
{
    // single thread = implicit strand
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


bool GlobeViewer::Impl::initGl() const
{
    makeCurrent();
    return gladLoadGL();
}


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
    mapGenerator->requestTiles.connect( std::bind( &TileManager::requestTiles, tileManager, ph::_1 ) );
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


bool GlobeViewer::validSetup() const
{
    return impl_ ? impl_->valid : false;
}


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


void GlobeViewer::resize( int w, int h )
{
    impl_->ioc.post( [this, w, h] {
        if ( impl_ )
        {
            impl_->viewport->resize( w, h );
        }
    } );
}


void GlobeViewer::move( int x, int y )
{
    impl_->ioc.post( [this, x, y] {
        if ( impl_ )
        {
            impl_->viewport->move( x, y );
        }
    } );
}


void GlobeViewer::zoom( int steps )
{
    impl_->ioc.post( [this, steps] {
        if ( impl_ )
        {
            impl_->viewport->zoom( steps );
        }
    } );
}


void GlobeViewer::centerView()
{
    impl_->ioc.post( [this] {
        if ( impl_ )
        {
            impl_->viewport->center();
        }
    } );
}


void GlobeViewer::baseState()
{
    impl_->ioc.post( [this] {
        if ( impl_ )
        {
            impl_->dataKeeper->balanceGlobe();
        }
    } );
}


void GlobeViewer::rotate( int x, int y )
{
    impl_->ioc.post( [this, x, y] {
        if ( impl_ )
        {
            impl_->dataKeeper->rotateGlobe( x, y );
        }
    } );
}


void GlobeViewer::projectCenterAt( int x, int y )
{
    impl_->ioc.post( [this, x, y] {
        if ( impl_ )
        {
            impl_->dataKeeper->centerAt( x, y );
        }
    } );
}


void GlobeViewer::cleanup()
{
    impl_.reset();
}


}
