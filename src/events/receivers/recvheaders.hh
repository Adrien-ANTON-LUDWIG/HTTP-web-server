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
        explicit RecvHeadersEW(const shared_connection &connection)
            : EventWatcher(connection->sock->fd_get()->fd_, EV_READ)
            , connection_(connection)
            , request_()
        {}

        /**
         * \brief Start or resume receiving data from the corresponding client.
         */
        void operator()() final;

    private:
        /**
         * @brief Structure connection
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
