#include "timer.hpp"

namespace wapstart {

    Timer::Timer(const std::string& msg):
        msg_(msg),
        startTime_(boost::posix_time::microsec_clock::local_time())
    {
    }

    void Timer::show()
    {
        __LOG_DEEP_DEBUG
                << msg_
                << " in "
                << boost::posix_time::microsec_clock::local_time() - startTime_;
    }
}
