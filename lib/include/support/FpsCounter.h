#pragma once

#include <ctime>


namespace gv {


/*!
 * \brief Simple frame counter.
 */
class FpsCounter
{
public:
    //! Default constructor.
    FpsCounter();

    //! Add one frame to total number.
    int addFrame();

private:
    std::time_t tBase_;     //!< Last marked timestamp.
    std::time_t tCurr_;     //!< Current timestamp.
    int frames_;            //!< Current number of frames counted.
};


}
