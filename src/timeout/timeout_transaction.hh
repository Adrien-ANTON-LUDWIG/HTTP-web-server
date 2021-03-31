#pragma once
#include <ev.h>

#include "events/events.hh"

namespace http
{
    class EventWatcher;

    class TimeoutTransaction
    {
    public:
        TimeoutTransaction(EventWatcher *ew, unsigned timeout);

        static void callback(struct ev_loop *, ev_timer *et, int);

        std::shared_ptr<ev_timer> get_et();

    private:
        std::shared_ptr<ev_timer> et_ = nullptr;
        EventWatcher *shared_ew_ = nullptr;
    };
} // namespace http