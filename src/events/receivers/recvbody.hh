/**
 * \file events/recvrequest.hh
 * \brief RecvBodyEW declaration.
 */

#pragma once

#include <arpa/inet.h>
#include <iostream>
#include <memory>

#include "events/events.hh"
#include "events/register.hh"
#include "misc/define.hh"
#include "vhost/dispatcher.hh"

namespace http
{
    /**
     * \class RecvBodyEW
     * \brief Workflow for Receive Headers events.
     */
    class RecvBodyEW : public EventWatcher
    {
    public:
        /**
         * @brief
         *
         */
        explicit RecvBodyEW(shared_connection &connection,
                            std::shared_ptr<Request> request)
            : EventWatcher(connection->sock->fd_get()->fd_, EV_READ)
        {
            connection_ = connection;
            request_ = request;

            if (connection->timeout_transaction != nullptr)
                connection->timeout_transaction->set_ew(this);
            if (connection->timeout_throughput != nullptr)
                connection->timeout_throughput->set_ew(this);
        }

        void unregister_timeout();

        /**
         * \brief Start or resume receiving data from the corresponding client.
         */
        void operator()() final;
    };
} // namespace http
