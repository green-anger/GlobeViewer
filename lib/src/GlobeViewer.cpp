#include <stdexcept>

#include "DataKeeper.h"
#include "GlobeViewer.h"
#include "Renderer.h"
#include "Viewport.h"


namespace gw
{


    struct GlobeViewer::Impl
    {
    public:
        Impl();
        ~Impl();

        bool initGl() const;
        void initData();

        bool valid;
        std::shared_ptr<DataKeeper> dataKeeper;
        std::shared_ptr<Viewport> viewport;
        std::unique_ptr<Renderer> renderer;
    };


    GlobeViewer::Impl::Impl()
        : valid( false )
    {
        if ( !initGl() )
            throw std::logic_error( "OpenGL initialization failed!" );

        initData();
    }


    GlobeViewer::Impl::~Impl()
    {
    }


    bool GlobeViewer::Impl::initGl() const
    {
        return gladLoadGL();
    }


    void GlobeViewer::Impl::initData()
    {
        dataKeeper.reset( new DataKeeper() );
        viewport.reset( new Viewport() );
        renderer.reset( new Renderer( dataKeeper, viewport ) );
        valid = true;
    }
    

    GlobeViewer::GlobeViewer()
        : impl_( new GlobeViewer::Impl )
    {
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
        impl_->renderer->render();
    }


}
