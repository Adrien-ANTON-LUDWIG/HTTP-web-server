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
        void operator()() final
        {
            char buffer[BUFFER_SIZE];
            auto read_size = connection_->sock->recv(buffer, BUFFER_SIZE);
            request_.body.append(buffer, buffer + read_size);

            if (request_.content_length <= request_.body.size())
            {
#ifdef _DEBUG
                std::cout << request_.body << std::endl;
#endif
                dispatcher.dispatch(connection_, request_);
                event_register.unregister_ew(this);
            }
            else if (read_size <= 0)
            {
                event_register.unregister_ew(this);
                std::cerr << "Connection closed (receive)\n";
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
