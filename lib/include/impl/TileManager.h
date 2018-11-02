#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/signals2.hpp>

#include "type/TileMap.h"
#include "TileServerFactory.h"


namespace gv {


/*!
 * \brief Downloads and caches map tiles.
 *
 * TileManager gets requests from the system and fulfils them one at a time.
 * For each requested tile a Session created to fetch the tile either from
 * cache or tile server. However they're not created straight away but put
 * in a queue which allows to reduce system load. Once all the requested tiles
 * have been fetched TileManager sends them to the system. It has no knowledge of
 * particular tile servers data, but uses a pointer to abstract TileServerBase
 * which can be implemented in any way.
 */
class TileManager
{
public:
    TileManager();
    ~TileManager();

    //! Start or queue a request to get tiles from a particular tile server.
    void requestTiles( const std::vector<TileHead>&, TileServer );

    //! Send tile images.
    boost::signals2::signal<void( const std::vector<TileImage>& )> sendTiles;

private:
    class Session;
    friend class Session;

    //! Create a directory for the tile.
    void prepareDir( const TileHead& );

    boost::asio::io_context ioc_;                       //!< Allows implementing task queue.
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_; //!< Provides work to ioc and doesn't let it stop.
    std::vector<std::thread> threads_;                  //!< Vector of worker thread.

    TileServer serverType_;                             //!< Type of tile server.
    std::unique_ptr<TileServerBase> tileServer_;        //!< Pointer to an actual tile server information.

    std::vector<TileImage> vecResult_;                  //!< Vector of tile images to be send in the end.
    std::mutex mutexResult_;                            //!< Allows to synchronize sessions adding to the result container.
    std::unique_ptr<std::promise<void>> promiseReady_;  //!< Allows to synchronize sessions reporting they are done.
    std::atomic<int> remains_;                          //!< Number of tiles that have yet to be fetched.

    std::mutex mutexState_;                             //!< Allows to synchronize requests from the system.
    bool activeRequest_;                                //!< Indicator of new request.
    bool pendingRequest_;                               //!< Indicator of pending request.
    std::vector<TileHead> lastRequest_;                 //!< Tiles header from the latest pending request.
    TileServer lastTileServer_;                         //!< Tile server from the latest pending request.

    std::mutex mutexCount_;                             //!< Allows to synchronize statistics container.
    std::unordered_map<std::string, int> mirrorCount_;  //!< Statistics container maps tile server mirror name to number of request to this mirror.
    std::atomic<int> cacheCount_;                       //!< Number of tiles fetched from the cache in the last request.
};


}
