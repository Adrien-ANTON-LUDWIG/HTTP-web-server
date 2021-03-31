#pragma once
#include <ev.h>

#include "events/events.hh"

namespace http
{
    class EventWatcher;

    class TimeoutThroughput
    {
    public:
        TimeoutThroughput(EventWatcher *ew, unsigned timeout,
                          unsigned int throughput_val);

        static void callback(struct ev_loop *, ev_timer *et, int);

        std::shared_ptr<ev_timer> get_et();

        void received_bytes(const unsigned int &bytes);

    private:
        std::shared_ptr<ev_timer> et_ = nullptr;
        EventWatcher *shared_ew_ = nullptr;
        unsigned int throughput_val_ = 0;
        unsigned int received_bytes_ = 0;
    };
} // namespace http