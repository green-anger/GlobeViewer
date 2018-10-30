#pragma once

#include <functional>
#include <memory>

#include "type/TileServer.h"


namespace gv {


/*!
 * \brief One and only API class to be included into a project to get GlobeViewer going.
 * 
 * The class is built using PIMPL technique. The implementation is hidden in struct Impl.
 * The class is thread safe: any API method can be called from any thread, it will be redirected
 * to a separate OpenGL thread. That's why the caller must "abandon" its OpenGL context (make it non-current)
 * and provide a callable to the GlobeViewer constructor so that it can make the context current.
 * This little inconvenience allows to run all OpenGL code in a separate thread in non-conflicting
 * manner with user code, for instance, GUI application.
 */
class GlobeViewer
{
public:
    GlobeViewer( std::function<void()> );
    ~GlobeViewer();

    //! Check if GlobeViewer ready to used.
    bool validSetup() const;

    //! Render OpenGL context.
    void render();

    //! Resize the view.
    void resize( int w, int h );

    //! Move the view.
    void move( int x, int y );

    //! Change the Globe scale.
    void zoom( int steps );

    //! Place the Globe in the view center.
    void centerView();

    //! Rotate the Globe back to the initial state.
    void baseState();

    //! Rotate the Globe.
    void rotate( int x, int y );

    //! Center projection at the particular view point.
    void projectCenterAt( int x, int y );

    //! Change source of map tiles.
    void setTileSource( TileServer );

    //! Turn wire-frame on or off.
    void setWireFrameView( bool );
    
    //! Turn map tiles on or off.
    void setMapTilesView( bool );

    //! Optional cleanup.
    void cleanup();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;    //!< Pointer to implementation.
};


}
