#pragma once
#include <ev.h>

#include "events/events.hh"

namespace http
{
    class EventWatcher;

    class Timeout
    {
    public:
        Timeout(EventWatcher *ew, unsigned timeout,
                void (*callback)(struct ev_loop *, ev_timer *, int));

        Timeout(EventWatcher *ew, unsigned timeout, unsigned int throughput_val,
                void (*callback)(struct ev_loop *, ev_timer *, int));

        static void keep_alive_cb(struct ev_loop *, ev_timer *et, int);
        static void transaction_cb(struct ev_loop *, ev_timer *et, int);
        static void throughput_cb(struct ev_loop *, ev_timer *et, int);
        static void proxy_transaction_cb(struct ev_loop *, ev_timer *et, int);

        std::shared_ptr<ev_timer> get_et();

        void set_ew(EventWatcher *ew)
        {
            shared_ew_ = ew;
        }

        void received_bytes(const unsigned int &bytes);

    private:
        std::shared_ptr<ev_timer> et_;
        EventWatcher *shared_ew_ = nullptr;
        unsigned int throughput_val_ = 0;
        unsigned int received_bytes_ = 0;
    };
} // namespace http