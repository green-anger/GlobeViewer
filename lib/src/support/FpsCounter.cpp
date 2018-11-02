#include "FpsCounter.h"


namespace gv {


/*!
 * Mark base timestamp to current time. Set frame number to zero.
 */
FpsCounter::FpsCounter()
    : tBase_( time( 0 ) )
    , frames_( 0 )
{
}


/*!
 * \return Number of frames if at least one second elapsed since the last
 * marked timestamp. Zero - otherwise.
 *
 * When a non-zero number returns, base timestamp is set to the current one and
 * frame number is set to zero.
 */
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
