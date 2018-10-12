#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/filesystem.hpp>

#include "TileManager.h"


using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
using namespace boost::filesystem;


namespace {


const std::string cache = "cache";

std::string dirForTile( const gv::TileHead& tileHead)
{
    return cache + "/" + std::to_string( tileHead.z ) + "/" + std::to_string( tileHead.x ) + "/";
}


}


namespace gv {


class TileManager::Session : public std::enable_shared_from_this<TileManager::Session>
{
public:
    explicit Session( boost::asio::io_context& ioc, const TileHead& );
    ~Session();

    void get( const std::string& host, const std::string& port );

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
    TileHead tileHead_;
    std::string target_;
};


}


namespace gv {


TileManager::TileManager()
    : ioc_()
    , work_( make_work_guard( ioc_ ) )
    , hosts_( { "a.tile.openstreetmap.org", "b.tile.openstreetmap.org", "c.tile.openstreetmap.org" } )
    , port_( "80" )
    , curHost_( 0 )
{
    std::cout << std::this_thread::get_id() << ": TileManager()" << std::endl;

    const int tNum = 4; // std::thread::hardware_concurrency() - 1;

    for ( int i = 0; i < tNum; ++i )
    {
        threads_.emplace_back( [this]() { ioc_.run(); } );
    }

    heads_.emplace( 0, 0, 0 );
    heads_.emplace( 1, 0, 0 );
    heads_.emplace( 1, 0, 1 );
    heads_.emplace( 1, 1, 0 );
    heads_.emplace( 1, 1, 1 );
    heads_.emplace( 2, 0, 0 );
    heads_.emplace( 2, 0, 1 );
    heads_.emplace( 2, 0, 2 );
    heads_.emplace( 2, 0, 3 );
    heads_.emplace( 2, 1, 0 );
    heads_.emplace( 2, 1, 1 );
    heads_.emplace( 2, 1, 2 );
    heads_.emplace( 2, 1, 3 );
    heads_.emplace( 2, 2, 0 );
    heads_.emplace( 2, 2, 1 );
    heads_.emplace( 2, 2, 2 );
    heads_.emplace( 2, 2, 3 );
    heads_.emplace( 2, 3, 0 );
    heads_.emplace( 2, 3, 1 );
    heads_.emplace( 2, 3, 2 );
    heads_.emplace( 2, 3, 3 );

    for ( const auto& head : heads_ )
    {
        prepareDir( head );
        tileMap_.emplace( std::piecewise_construct, std::make_tuple( head ), std::make_tuple() );
        //tileMap_.emplace( std::piecewise_construct, std::make_tuple( 1, 0, 0 ), std::make_tuple() );
    }

    download( tileMap_ );
}


TileManager::~TileManager()
{
    work_.reset();
    ioc_.stop();

    for ( auto&& t : threads_ )
    {
        if ( t.joinable() )
        {
            t.join();
        }
    }
}


void TileManager::download( Tile& tile )
{
    std::make_shared<Session>( ioc_, tile.head )->get( hosts_[curHost_], port_ );
    curHost_ = ( curHost_ + 1 ) % std::tuple_size_v<decltype( hosts_ )>;
}


void TileManager::download( TileMap& tileMap )
{
    TileMap ts;

    for ( auto& t : tileMap )
    {
        Tile tile( t.first, t.second );
        download( tile );
        t.second = std::move( tile.body );
    }
}


void TileManager::cache( Tile& tile ) const
{
}


void TileManager::cache( TileMap& tileMap ) const
{
}


bool TileManager::cacheHas( const Tile& tile ) const
{
    return false;
}


void TileManager::prepareDir( const TileHead& tileHead )
{
    auto key = std::make_pair( tileHead.z, tileHead.x );
    auto it = dirs_.find( key );

    if ( it == dirs_.end() )
    {
        auto dir = dirForTile( tileHead );
        create_directories( dir );
        dirs_.emplace( key, dir );
    }
}


}


namespace gv {


TileManager::Session::Session( boost::asio::io_context& ioc, const TileHead& head )
    : resolver_( ioc )
    , socket_( ioc )
    , tileHead_( head )
{
    target_ =
        "/" + std::to_string( head.z ) +
        "/" + std::to_string( head.x ) +
        "/" + std::to_string( head.y ) + ".png";
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


void TileManager::Session::get( const std::string& host, const std::string& port )
{
    request_.version( 11 );
    request_.method( http::verb::get );
    request_.target( target_ );
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

    const auto& body = response_.body();
    std::ofstream tile( dirForTile( tileHead_ ) + std::to_string( tileHead_.y ) + ".png", std::ios::out | std::ios::binary );
    tile.write( body.c_str(), body.size() );
    tile.close();

    std::cout << "Tile " << target_ << " was written!" << std::endl;
}


}
