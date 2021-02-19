/**
 * \file events/recvrequest.hh
 * \brief RecvRequestEW declaration.
 */

#pragma once

#include <arpa/inet.h>

#include "events/events.hh"
#include "vhost/dispatcher.hh"

#define BUFFER_SIZE 512

namespace http
{
    /**
     * \class RecvHeadersEW
     * \brief Workflow for Receive Headers events.
     */
    class RecvRequestEW : public EventWatcher
    {
    public:
        /**
         * \brief Create a RecvRequestEW from a listener socket.
         */
        explicit RecvRequestEW(shared_socket socket)
            : EventWatcher(socket->fd_get()->fd_, EV_READ)
        {
            sock_ = socket;
        }

        /**
         * \brief Start or resume receiving data from the corresponding client.
         */
        void operator()() final
        {
            char buffer[BUFFER_SIZE];
            auto read_size = sock_->recv(buffer, BUFFER_SIZE);
            message.append(buffer, buffer + read_size);
            std::string carriage = "\r\n\r\n";

            if (message.find(carriage) == message.size() - carriage.size())
            {
                std::cout << message;
                // Process
                // dispatcher.dispatch(this);

                message.erase();
            }
        }

    private:
        /**
         * \brief Client socket.
         */
        shared_socket sock_;

        /**
         * \brief Message of the client
         */
        std::string message = "";

        /**
         * @brief
         *
         */
        Connection connection;
    };
} // namespace http
