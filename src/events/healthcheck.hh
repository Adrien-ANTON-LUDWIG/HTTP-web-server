/**
 * \file events/listener.hh
 * \brief ListenerEW declaration.
 */

#pragma once

#include <arpa/inet.h>

#include "events/event-loop.hh"
#include "events/events.hh"
#include "events/receivers/recvheaders.hh"
#include "events/register.hh"
#include "socket/socket.hh"

namespace http
{
    /**
     * \class HealthCheckEW
     * \brief Workflow for listener socket.
     */
    class HealthCheckEW
    {
    public:
        /**
         * \brief Create a HealthCheckEW from a listener socket.
         */
        HealthCheckEW(const Backend &backend);

        ~HealthCheckEW();

        void timeout_cb(EV_P_ ev_timer *w, int revents);

        // static void callback(EV_P_ ev_timer *w, int revents)
        void check(struct ev_loop *, ev_timer *, int revents);

    private:
        Backend backend_;
        ev_timer et_;
    };
} // namespace http
