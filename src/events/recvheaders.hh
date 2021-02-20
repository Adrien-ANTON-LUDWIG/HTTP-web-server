/**
 * \file events/recvrequest.hh
 * \brief RecvHeadersEW declaration.
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
        explicit RecvHeadersEW(const struct Connection &connection)
            : EventWatcher(connection.sock_->fd_get()->fd_, EV_READ)
            , connection_(connection)
            , request_()
        {}

        /**
         * \brief Start or resume receiving data from the corresponding client.
         */
        void operator()() final
        {
            char buffer[BUFFER_SIZE];
            auto read_size = connection_.sock_->recv(buffer, BUFFER_SIZE);
            connection_.message.append(buffer, buffer + read_size);
            std::string carriage = "\r\n\r\n";

            if (connection_.message.find(carriage)
                == connection_.message.size() - carriage.size())
            {
                std::cout << connection_.message;
                // Process

                request_.parse_headers(connection_.message);
                request_.pretty_print();

                // Parse request
                // If body needed
                //  Create a RecvBodyEW with connection and request
                // Else
                // dispatcher.dispatch(this);
                // Unregister this event

                connection_.message.erase();
            }
        }

    private:
        /**
         * @brief
         *
         */
        Connection connection_;

        /**
         * @brief Structure to contain the parsed request.
         *
         */
        struct Request request_;
    };
} // namespace http
