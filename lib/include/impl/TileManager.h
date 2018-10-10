#pragma once

#include <thread>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>


namespace gv {


class TileManager
{
public:
    TileManager();
    ~TileManager();

private:
    class Session;
  
    boost::asio::io_context ioc;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work;
    std::vector<std::thread> threads;
};


}
