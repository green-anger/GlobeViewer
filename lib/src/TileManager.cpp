#include <atomic>
#include <chrono>
#include <fstream>
#include <future>
#include <memory>
#include <string>
#include <tuple>

#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/filesystem.hpp>

#include "TileManager.h"
#include "ThreadSafePrinter.hpp"


using tcp = boost::asio::ip::tcp;
using steady_timer = boost::asio::steady_timer;
namespace http = boost::beast::http;
using namespace boost::filesystem;
using TSP = alt::ThreadSafePrinter<alt::MarkPolicy>;

namespace {


const std::string cache = "cache";

std::string tileDir( const gv::TileHead& head, const std::string& name )
{
    return cache + "/" + name + "/" + std::to_string( head.z ) + "/" + std::to_string( head.x ) + "/";
}

std::string tileFile( const gv::TileHead& head, const std::string& name )
{
    return tileDir( head, name ) + std::to_string( head.y ) + ".png";
}


}


namespace gv {


class TileManager::Session : public std::enable_shared_from_this<TileManager::Session>
{
public:
    explicit Session( TileManager*, const TileHead&, int msecTimeout );
    ~Session();

    void start();

private:
    void stop();
    void error( boost::system::error_code, const std::string& );
    void get( const std::string& host, const std::string& port );
    void onTimeout();
    void onResolve( boost::system::error_code, tcp::resolver::results_type );
    void onConnect( boost::system::error_code );
    void onWrite( boost::system::error_code, std::size_t );
    void onRead( boost::system::error_code, std::size_t );

    void checkRemains();

    TileManager* tm_;
    TileHead tileHead_;
    std::string mirror_;

    steady_timer timer_;
    tcp::resolver resolver_;
    tcp::socket socket_;
    boost::beast::flat_buffer buffer_;
    http::request<http::empty_body> request_;
    http::response<http::string_body> response_;

    std::chrono::time_point<std::chrono::steady_clock> start_;
    int tries_;
    const int msecTimeout_;
    std::atomic<bool> connected_;
};


}


namespace gv {


TileManager::TileManager()
    : ioc_()
    , work_( make_work_guard( ioc_ ) )
    , serverType_( TileServer::OSM )
    , tileServer_( TileServerFactory::createTileServer( serverType_ ) )
    , activeRequest_( false )
    , pendingRequest_( false )
{
    const int tNum = 4; // std::thread::hardware_concurrency() - 1;

    for ( int i = 0; i < tNum; ++i )
    {
        threads_.emplace_back( [this]() { ioc_.run(); } );
    }
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


void TileManager::requestTiles( const std::vector<TileHead>& vec, TileServer ts )
{
    TSP() << "Request for " << vec.size() << " tiles";

    if ( vec.empty() )
    {
        vecResult_.clear();
        sendTiles( vecResult_ );
        return;
    }

    {   // checking states
        std::lock_guard<std::mutex> lock( mutexState_ );

        if ( activeRequest_ )
        {
            pendingRequest_ = true;
            lastRequest_ = std::move( vec );
            lastTileServer_ = ts;
            return;
        }
        else
        {
            activeRequest_ = true;
        }
    }
    
    if ( ts != serverType_ )
    {
        serverType_ = ts;
        tileServer_ = TileServerFactory::createTileServer( serverType_ );
    }

    vecResult_.clear();
    remains_ = vec.size();
    promiseReady_.reset( new std::promise<void> );

    ioc_.post( [this, rem = vec.size()]()
    {
        promiseReady_->get_future().wait();

        TSP() << "All " << rem << " jobs are done!\n"
            << "From cache: " << cacheCount_ << "\n"
            << "From mirrors" << ( mirrorCount_.empty() ? " nothing" : ": " );

        for ( const auto& it : mirrorCount_ )
        {
            TSP() << it.first << ": " << it.second;
        }

        sendTiles( vecResult_ );

        std::lock_guard<std::mutex> lock( mutexState_ );
        activeRequest_ = false;

        if ( pendingRequest_ )
        {
            pendingRequest_ = false;
            // otherwise mutexState_ will be locked twice which is UB
            ioc_.post( [this] { requestTiles( lastRequest_, lastTileServer_ ); } );
            ///\todo this request can arrive later than a new one from outside
            /// which will make it outdated, some check needs to be added to prevent
            /// processing of uotdated requests
        }
    } );

    mirrorCount_.clear();
    cacheCount_ = 0;

    for ( const auto& head : vec )
    {
        ioc_.post( [head, this]()
        {
            std::make_shared<Session>( this, head, 1000 )->start();
        } );
    }
}


void TileManager::prepareDir( const TileHead& head )
{
    create_directories( tileDir( head, tileServer_->serverName() ) );
}


}


namespace gv {


TileManager::Session::Session( TileManager* tm, const TileHead& head, int msecTimeout )
    : tm_( tm )
    , tileHead_( head )
    , timer_( tm_->ioc_ )
    , resolver_( tm_->ioc_ )
    , socket_( tm_->ioc_ )
    , tries_( 3 )
    , msecTimeout_( msecTimeout )
    , connected_( false )
{
}


TileManager::Session::~Session()
{
    stop();
    checkRemains();
}


void TileManager::Session::start()
{
    const auto tilePath = tileFile( tileHead_, tm_->tileServer_->serverName() );

    if ( exists( path( tilePath ) ) )
    {
        std::ifstream tile( tilePath.c_str(), std::ios::in | std::ios::binary );
        std::vector<unsigned char> vec{ std::istreambuf_iterator<char>( tile ), std::istreambuf_iterator<char>() };
        tile.close();

        {
            std::lock_guard<std::mutex> lock( tm_->mutexResult_ );
            tm_->vecResult_.emplace_back( tileHead_, std::move( TileData( vec ) ) );
        }

        ++tm_->cacheCount_;
    }
    else
    {
        mirror_ = tm_->tileServer_->nextMirror();
        get( mirror_, tm_->tileServer_->serverPort() );
    }
}


void TileManager::Session::get( const std::string& host, const std::string& port )
{
    start_ = std::chrono::steady_clock::now();

    request_.version( 11 );
    request_.method( http::verb::get );
    request_.target( tm_->tileServer_->tileTarget( tileHead_.z, tileHead_.x, tileHead_.y ) );
    request_.set( http::field::host, host.c_str() );
    request_.set( http::field::user_agent, BOOST_BEAST_VERSION_STRING );

    namespace ph = std::placeholders;
    resolver_.async_resolve( host.c_str(), port.c_str(),
        std::bind( &Session::onResolve, shared_from_this(), ph::_1, ph::_2 ) );
}


void TileManager::Session::onTimeout()
{
    if ( !connected_ )
    {
        stop();
    }
}


void TileManager::Session::stop()
{
    if ( socket_.is_open() )
    {
        boost::system::error_code ec;

        socket_.shutdown( tcp::socket::shutdown_both, ec );

        if ( ec && ec != boost::system::errc::not_connected )
        {
            TSP() << "Session shutdown error: " << ec.message();
        }

        socket_.close( ec );

        if ( ec )
        {
            TSP() << "Session socket close error: " << ec.message();
        }
    }
}


void TileManager::Session::error( boost::system::error_code ec, const std::string& msg )
{
    TSP() << msg << ": " << ec.message() << "\n";

    if ( --tries_ > 0 && boost::system::errc::host_unreachable == ec )
    {
        start();
    }
}


void TileManager::Session::onResolve( boost::system::error_code ec, tcp::resolver::results_type results )
{
    if ( ec )
    {
        return error( ec, "resolve" );
    }

    namespace ph = std::placeholders;
    timer_.expires_at( std::chrono::steady_clock::now() + std::chrono::milliseconds( msecTimeout_ ) );
    timer_.async_wait( std::bind( &TileManager::Session::onTimeout, shared_from_this() ) );
    boost::asio::async_connect( socket_, results.begin(), results.end(),
        std::bind( &Session::onConnect, shared_from_this(), ph::_1 ) );
}


void TileManager::Session::onConnect( boost::system::error_code ec )
{
    if ( ec )
    {
        return error( ec, "connect" );
    }

    connected_ = true;
    timer_.cancel();
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

    //auto end = std::chrono::steady_clock::now();
    //auto msec = std::chrono::duration_cast< std::chrono::milliseconds >( end - start_ ).count();

    //auto head = response_.base();

    //TSP() << "Got response in " << msec << " msecs\n"
    //    << "result = " << head.result() << "\n"
    //    << "version = " << head.version() << "\n"
    //    << "reason = " << head.reason() << "\n";

    tm_->prepareDir( tileHead_ );
    const auto& body = response_.body();
    std::ofstream tile( tileFile( tileHead_, tm_->tileServer_->serverName() ), std::ios::out | std::ios::binary );
    tile.write( body.c_str(), body.size() );
    tile.close();

    std::vector<unsigned char> vec( body.begin(), body.end() );
    {
        std::lock_guard<std::mutex> lock( tm_->mutexResult_ );
        tm_->vecResult_.emplace_back( std::move( tileHead_ ), std::move( vec ) );
    }

    {
        std::lock_guard<std::mutex> lock( tm_->mutexCount_ );

        auto it = tm_->mirrorCount_.find( mirror_ );

        if ( it != tm_->mirrorCount_.end() )
        {
            ++it->second;
        }
        else
        {
            tm_->mirrorCount_.emplace( mirror_, 1 );
        }
    }
}


void TileManager::Session::checkRemains()
{
    if ( --tm_->remains_ == 0 )
    {
        tm_->promiseReady_->set_value();
    }
}


}
