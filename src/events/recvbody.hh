/**
 * \file events/recvrequest.hh
 * \brief RecvBodyEW declaration.
 */

#pragma once

#include <arpa/inet.h>
#include <iostream>

#include "events/events.hh"
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
        explicit RecvBodyEW(const struct Connection &connection,
                            const struct Request &request)
            : EventWatcher(connection.sock_->fd_get()->fd_, EV_READ)
            , request(request)
        {}

        /**
         * \brief Start or resume receiving data from the corresponding client.
         */
        void operator()() final
        {
            char buffer[BUFFER_SIZE];
            auto read_size = connection.sock_->recv(buffer, BUFFER_SIZE);
            connection.message.append(buffer, buffer + read_size);
            std::string carriage = "\r\n\r\n";

            if (connection.message.find(carriage)
                == connection.message.size() - carriage.size())
            {
                std::cout << connection.message;
                // Process
                // dispatcher.dispatch(this);

                connection.message.erase();
            }
        }

    private:
        /**
         * @brief
         *
         */
        Connection connection;

        /**
         * @brief Structure to contain the parsed request.
         *
         */
        struct Request request;
    };
} // namespace http
