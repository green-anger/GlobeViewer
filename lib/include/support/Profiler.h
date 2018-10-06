#pragma once

#include <chrono>
#include <map>
#include <iostream>
#include <string>

#include <vector>


namespace gv {
namespace support {


class Profiler
{
public:
    Profiler( const std::string& name, bool print = false, std::ostream& = std::cout );
    ~Profiler();

    static void printStatistics( std::ostream& = std::cout );
    static void getStatistics( std::ostream& );
    static std::string getStatistics();

private:
    static void addStatistic( const std::string&, int );
    std::size_t currentMilli() const;

    const std::string name_;
    bool print_;
    std::ostream& out_;
    std::chrono::time_point<std::chrono::steady_clock> tick_;

    // name -> (total time, count)
    static std::map<std::string, std::pair<int, int>> map_;
};


}
}
