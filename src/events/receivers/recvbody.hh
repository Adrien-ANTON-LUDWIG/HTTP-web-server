/**
 * \file events/recvrequest.hh
 * \brief RecvBodyEW declaration.
 */

#pragma once

#include <arpa/inet.h>
#include <iostream>

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
                            const struct Request &request)
            : EventWatcher(connection->sock->fd_get()->fd_, EV_READ)
            , connection_(connection)
            , request_(request)
        {}

        /**
         * \brief Start or resume receiving data from the corresponding client.
         */
        void operator()() final;

    private:
        /**
         * @brief
         *
         */
        shared_connection connection_;

        /**
         * @brief Structure to contain the parsed request.
         *
         */
        struct Request request_;
    };
} // namespace http
