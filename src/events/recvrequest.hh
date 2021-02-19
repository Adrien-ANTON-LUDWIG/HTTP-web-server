/**
 * \file events/recvrequest.hh
 * \brief RecvRequestEW declaration.
 */

#pragma once

#include <arpa/inet.h>

#include "events/events.hh"

namespace http
{
    /**
     * \class RecvRequestEW
     * \brief Workflow for Receive Request events.
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
            return;
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
    };
} // namespace http
