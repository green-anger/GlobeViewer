#pragma once

#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <utility>


namespace alt
{


    class NullPolicy
    {
    public:
        static void prefix( std::ostringstream& oss ) {}
        static void suffix( std::ostringstream& oss ) {}
    };


    class MarkPolicy
    {
    public:
        static void prefix( std::ostringstream& oss )
        {
            oss << "[" << std::this_thread::get_id() << "] ";
        }

        static void suffix( std::ostringstream& oss ) {}
    };


    class MutexKeeper
    {
        template<typename> friend class ThreadSafePrinter;

        static std::mutex& mutex()
        {
            static std::mutex mutex;
            return mutex;
        }
    };


    template<typename Policy = NullPolicy>
    class ThreadSafePrinter
    {
    public:
        ThreadSafePrinter( std::ostream& out = std::cout )
            : out_( out )
        {
            Policy::prefix( oss_ );
        }

        ~ThreadSafePrinter()
        {
            Policy::suffix( oss_ );
            std::lock_guard<std::mutex> lock( MutexKeeper::mutex() );
            out_ << oss_.str() << std::endl;
        }

        template<typename T>
        inline ThreadSafePrinter& operator<<( T&& t )
        {
            oss_ << std::forward<T>( t );
            return *this;
        }

    private:
        std::ostringstream oss_;
        std::ostream& out_;
    };


}