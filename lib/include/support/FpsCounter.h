#pragma once

#include <ctime>


namespace gv {


class FpsCounter
{
public:
    FpsCounter();
    int addFrame();

private:
    std::time_t tBase_;
    std::time_t tCurr_;
    int frames_;
};


}
