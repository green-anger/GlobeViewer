#include <functional>
#include <future>
#include <stdexcept>
#include <thread>

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>

#include "DataKeeper.h"
#include "GlobeViewer.h"
#include "Renderer.h"
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
    std::shared_ptr<Viewport> viewport;
    std::unique_ptr<Renderer> renderer;

    boost::asio::io_context ioc;
    std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> work;
    std::vector<std::thread> threads;
};


GlobeViewer::Impl::Impl( std::function<void()> func )
    : valid( false )
    , makeCurrent( func )
    , ioc()
    , work( std::make_unique<executor_work_guard<io_context::executor_type>>( ioc.get_executor() ) ) 
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
    viewport.reset( new Viewport() );
    renderer.reset( new Renderer( dataKeeper, viewport ) );

    dataKeeper->registerUnitInMeterGrabber( std::bind( &Viewport::unitInMeter, viewport ) );
    dataKeeper->registerMeterInPixelGrabber( std::bind( &Viewport::meterInPixel, viewport ) );

    dataKeeper->init();

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
