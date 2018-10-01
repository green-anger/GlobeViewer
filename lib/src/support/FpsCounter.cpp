#include "FpsCounter.h"


namespace gv {


FpsCounter::FpsCounter()
    : tBase_( time( 0 ) )
    , frames_( 0 )
{
}

int FpsCounter::addFrame()
{
    tCurr_ = time( 0 );
    std::time_t diff = tCurr_ - tBase_;
    if ( diff > 0 )
    {
        int res = frames_;
        frames_ = 0;
        tBase_ = tCurr_;
        return res;
    }
    ++frames_;
    return 0;
}


}
