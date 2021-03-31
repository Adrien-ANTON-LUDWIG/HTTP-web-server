/**
 * \file events/recvrequest.hh
 * \brief RecvHeadersEW declaration.
 */

#pragma once

#include <arpa/inet.h>
#include <iostream>

#include "events/events.hh"
#include "events/receivers/recvbody.hh"
#include "misc/define.hh"
#include "vhost/dispatcher.hh"

namespace http
{
    /**
     * \class RecvHeadersEW
     * \brief Workflow for Receive Headers events.
     */
    class RecvHeadersEW : public EventWatcher
    {
    public:
        /**
         * @brief
         *
         */
        RecvHeadersEW(const shared_connection connection)
            : EventWatcher(connection->sock->fd_get()->fd_, EV_READ)
        {
            connection_ = connection;

            if (http::dispatcher.serv_config_.timeout_keepalive.has_value())
                connection_
                    ->timeout_keep_alive = std::make_shared<TimeoutKeepAlive>(
                    this,
                    http::dispatcher.serv_config_.timeout_keepalive.value());
        }

        void handle_timeout_begin();
        void unregister_timeout();

        /**
         * \brief Start or resume receiving data from the corresponding client.
         */
        void operator()() final;

    private:
        bool first_time_ = true;
    };

} // namespace http
