#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/png_io.hpp>

#include "TileManager.h"


using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
using namespace boost::gil;


namespace gv
{


class TileManager::Session : public std::enable_shared_from_this<TileManager::Session>
{
public:
    explicit Session( boost::asio::io_context& ioc );
    ~Session();

    void run( const std::string& host, const std::string& port, const std::string& target, int version );

private:
    void error( boost::system::error_code, const std::string& );
    void onResolve( boost::system::error_code, tcp::resolver::results_type );
    void onConnect( boost::system::error_code );
    void onWrite( boost::system::error_code, std::size_t );
    void onRead( boost::system::error_code, std::size_t );

    tcp::resolver resolver_;
    tcp::socket socket_;
    boost::beast::flat_buffer buffer_;
    http::request<http::empty_body> request_;
    http::response<http::string_body> response_;
    std::chrono::time_point<std::chrono::steady_clock> start_;
};


TileManager::TileManager()
    : ioc()
    , work( make_work_guard( ioc ) )
{
    std::cout << std::this_thread::get_id() << ": TileManager()" << std::endl;

    const int tNum = 2;

    for ( int i = 0; i < tNum; ++i )
    {
        threads.emplace_back( [this]() { ioc.run(); } );
    }

    const std::string host = "a.tile.openstreetmap.org";
    const std::string port = "80";
    const std::string target = "/0/0/0.png";
    int version = 11;

    std::make_shared<Session>( ioc )->run( host, port, target, version );
}


TileManager::~TileManager()
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


TileManager::Session::Session( boost::asio::io_context& ioc )
    : resolver_( ioc )
    , socket_( ioc )
{
}


TileManager::Session::~Session()
{
    boost::system::error_code ec;
    socket_.shutdown( tcp::socket::shutdown_both, ec );

    if ( ec && ec != boost::system::errc::not_connected )
    {
        error( ec, "shutdown" );
    }
}


void TileManager::Session::run( const std::string& host, const std::string& port, const std::string& target, int version )
{
    request_.version( version );
    request_.method( http::verb::get );
    request_.target( target );
    request_.set( http::field::host, host.c_str() );
    request_.set( http::field::user_agent, BOOST_BEAST_VERSION_STRING );

    start_ = std::chrono::steady_clock::now();

    namespace ph = std::placeholders;
    resolver_.async_resolve( host.c_str(), port.c_str(),
        std::bind( &Session::onResolve, shared_from_this(), ph::_1, ph::_2 ) );
}


void TileManager::Session::error( boost::system::error_code ec, const std::string& msg )
{
    std::cerr << msg << ": " << ec.message() << "\n";
}


void TileManager::Session::onResolve( boost::system::error_code ec, tcp::resolver::results_type results )
{
    if ( ec )
    {
        return error( ec, "resolve" );
    }

    namespace ph = std::placeholders;
    boost::asio::async_connect( socket_, results.begin(), results.end(),
        std::bind( &Session::onConnect, shared_from_this(), ph::_1 ) );
}


void TileManager::Session::onConnect( boost::system::error_code ec )
{
    if ( ec )
    {
        return error( ec, "connect" );
    }

    namespace ph = std::placeholders;
    http::async_write( socket_, request_,
        std::bind( &Session::onWrite, shared_from_this(), ph::_1, ph::_2 ) );
}


void TileManager::Session::onWrite( boost::system::error_code ec, std::size_t )
{
    if ( ec )
    {
        return error( ec, "write" );
    }

    namespace ph = std::placeholders;
    http::async_read( socket_, buffer_, response_,
        std::bind( &Session::onRead, shared_from_this(), ph::_1, ph::_2 ) );
}


void TileManager::Session::onRead( boost::system::error_code ec, std::size_t bytes_transferred )
{
    if ( ec )
    {
        return error( ec, "read" );
    }

    auto end = std::chrono::steady_clock::now();
    auto msec = std::chrono::duration_cast< std::chrono::milliseconds >( end - start_ ).count();

    auto head = response_.base();

    std::cout << "Got response in " << msec << " msecs\n"
        << "result = " << head.result() << "\n"
        << "version = " << head.version() << "\n"
        << "reason = " << head.reason() << "\n"
        << std::endl;

    rgb8_image_t img( 512, 512 );
    rgb8_pixel_t red( 255, 0, 0 );
    fill_pixels( view( img ), red );
    png_write_view( "red.png", const_view( img ) );

    std::cout << "Red square was written!" << std::endl;
}


}
