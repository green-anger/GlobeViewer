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


namespace gv {


class TileManager
{
public:
    TileManager();
    ~TileManager();

    void requestTiles( const std::vector<TileHead>& );
    boost::signals2::signal<void( const std::vector<TileImage>& )> sendTiles;

private:
    class Session;

    bool fromCache( Tile& );
    void prepareDir( const TileHead& );

    boost::asio::io_context ioc_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_;
    std::vector<std::thread> threads_;

    std::unordered_map<std::pair<int, int>, std::string> dirs_;
    const std::array<std::string, 3> hosts_;
    const std::string port_;
    std::atomic<int> curHost_;

    std::vector<TileImage> vecResult_;
    std::mutex mutexResult_;
    std::unique_ptr<std::promise<void>> promiseReady_;
    std::atomic<int> remains_;

    std::mutex mutexState_;
    bool activeRequest_;
    bool pendingRequest_;
    std::vector<TileHead> lastRequest_;
};


}
