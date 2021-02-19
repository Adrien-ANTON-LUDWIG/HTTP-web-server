/**
 * \file events/listener.hh
 * \brief ListenerEW declaration.
 */

#pragma once

#include <arpa/inet.h>

#include "events/events.hh"
#include "socket/socket.hh"

namespace http
{
    /**
     * \class ListenerEW
     * \brief Workflow for listener socket.
     */
    class ListenerEW : public EventWatcher
    {
    public:
        /**
         * \brief Create a ListenerEW from a listener socket.
         */
        explicit ListenerEW(shared_socket socket)
            : EventWatcher(socket->fd_get()->fd_, EV_READ)
        {
            sock_ = socket;
            struct sockaddr_in sin;
            socklen_t len = sizeof(sin);
            getsockname(socket->fd_get()->fd_, (struct sockaddr *)&sin, &len);
            port_ = ntohs(sin.sin_port);
        }

        /**
         * \brief Start accepting connections on listener socket.
         */
        void operator()() final
        {
            std::cout << "Client connected !\n";
        }

    private:
        /**
         * \brief Listener socket.
         */
        shared_socket sock_;
        /**
         * \brief Port on which the socket is listening.
         */
        uint16_t port_;
    };
} // namespace http
