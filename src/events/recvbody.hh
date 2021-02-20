/**
 * \file events/recvrequest.hh
 * \brief RecvBodyEW declaration.
 */

#pragma once

#include <arpa/inet.h>
#include <iostream>

#include "events/events.hh"
#include "events/register.hh"
#include "vhost/dispatcher.hh"

#define BUFFER_SIZE 512

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
            : EventWatcher(connection->sock_->fd_get()->fd_, EV_READ)
            , connection_(connection)
            , request_(request)
        {}

        /**
         * \brief Start or resume receiving data from the corresponding client.
         */
        void operator()() final
        {
            char buffer[BUFFER_SIZE];
            auto read_size = connection_->sock_->recv(buffer, BUFFER_SIZE);
            request_.body.append(buffer, buffer + read_size);

            if (request_.content_length == request_.body.size())
            {
                std::cout << request_.body;
                dispatcher.dispatch(connection_, request_);
                event_register.unregister_ew(this);
            }
        }

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
