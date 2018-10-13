#include <functional>
#include <future>
#include <stdexcept>
#include <thread>

#include <iostream>

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/signals2.hpp>

#include "DataKeeper.h"
#include "GlobeViewer.h"
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
}


bool GlobeViewer::Impl::initGl() const
{
    makeCurrent();
    return gladLoadGL();
}


void GlobeViewer::Impl::initData()
{
    dataKeeper.reset( new DataKeeper() );
    projector.reset( new Projector() );
    renderer.reset( new Renderer() );
    tileManager.reset( new TileManager() );
    viewport.reset( new Viewport() );

    namespace ph = std::placeholders;

    dataKeeper->getUnitInMeter.connect( std::bind( &Viewport::unitInMeter, viewport ) );
    dataKeeper->getMeterInPixel.connect( std::bind( &Viewport::meterInPixel, viewport ) );
    dataKeeper->getProjector.connect( [this]() -> auto { return projector; } );

    renderer->getProjection.connect( std::bind( &Viewport::projection, viewport ) );
    renderer->renderSimpleTriangle.connect( std::bind( &DataKeeper::simpleTriangle, dataKeeper ) );
    renderer->renderWireGlobe.connect( std::bind( &DataKeeper::wireGlobe, dataKeeper ) );

    tileManager->sendTiles.connect( []( const std::vector<TileImage>& vec )
    {
        std::cout << "For testing purpose only we get " << vec.size() << " tile images" << std::endl;
    } );

    dataKeeper->init();

    valid = true;

    {
        std::cout << "Requesting some tile images" << std::endl;
        //tileManager->requestTiles( { { 0, 0, 0} } );
        //tileManager->requestTiles( { { 0, 0, 0} , { 1, 0 ,0 }, { 1, 0 ,1 }, { 1, 1 ,0 }, { 1, 1 ,1 } } );
        tileManager->requestTiles( { { 0, 0, 0} , { 1, 0 ,0 }, { 1, 0 ,1 }, { 1, 1 ,0 }, { 1, 1 ,1 },
            { 2, 0, 0 }, { 2, 0 , 1 }, { 2, 0 ,2 }, { 2, 0, 3 },
            { 2, 1, 0 }, { 2, 1 , 1 }, { 2, 1 ,2 }, { 2, 1, 3 },
            { 2, 2, 0 }, { 2, 2 , 1 }, { 2, 2 ,2 }, { 2, 2, 3 },
            { 2, 3, 0 }, { 2, 3 , 1 }, { 2, 3 ,2 }, { 2, 3, 3 }
            } );
    }
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
    return impl_->valid;
}


void GlobeViewer::render()
{
    impl_->ioc.post( [this] {
        impl_->makeCurrent();
        impl_->renderer->render();
    } );
}


void GlobeViewer::resize( int w, int h )
{
    impl_->ioc.post( [this, w, h] { impl_->viewport->resize( w, h ); } );
}


void GlobeViewer::move( int x, int y )
{
    impl_->ioc.post( [this, x, y] { impl_->viewport->move( x, y ); } );
}


void GlobeViewer::zoom( int steps )
{
    impl_->ioc.post( [this, steps] { impl_->viewport->zoom( steps ); } );
}


void GlobeViewer::centerView()
{
    impl_->ioc.post( [this] { impl_->viewport->center(); } );
}


void GlobeViewer::baseState()
{
    impl_->ioc.post( [this] { impl_->dataKeeper->balanceGlobe(); } );
}


void GlobeViewer::rotate( int x, int y )
{
    impl_->ioc.post( [this, x, y] { impl_->dataKeeper->rotateGlobe( x, y ); } );
}


void GlobeViewer::cleanup()
{
    impl_.reset();
}


}
