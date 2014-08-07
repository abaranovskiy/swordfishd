#ifndef TIMER_HPP
#define TIMER_HPP

#include <boost/date_time/posix_time/posix_time.hpp>
#include "logger.hpp"

namespace wapstart {

    class Timer {
    public:
        Timer(const std::string&);
        void show();
    private:
        const std::string msg_;
        const boost::posix_time::ptime startTime_;
    };
}

#endif // TIMER_HPP
