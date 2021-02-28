/**
 * \file events/listener.hh
 * \brief ListenerEW declaration.
 */

#pragma once

#include <arpa/inet.h>

#include "events/events.hh"
#include "events/recvheaders.hh"
#include "events/register.hh"
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
            if (sin.sin_family == AF_INET)
            {
                char ipv4addr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &sin.sin_addr, ipv4addr, sizeof(ipv4addr));
                ip_ = std::string(ipv4addr);
            }
            else
            {
                char ipv6addr[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, &sin.sin_addr, ipv6addr, sizeof(ipv6addr));
                ip_ = std::string(ipv6addr);
            }
            port_ = ntohs(sin.sin_port);
        }

        /**
         * \brief Start accepting connections on listener socket.
         */
        void operator()() final
        {
            try
            {
                auto client_socket = sock_->accept(nullptr, nullptr);
                shared_connection connection =
                    std::make_shared<Connection>(client_socket, ip_, port_);
                event_register.register_event<RecvHeadersEW>(connection);
#ifdef _DEBUG
                std::cout << "Client connected !\n";
#endif
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error while accepting client: " << e.what()
                          << '\n';
                return;
            }
        }

    private:
        /**
         * \brief Listener socket.
         */
        shared_socket sock_;

        /**
         * @brief Ip of
         *
         */
        std::string ip_;

        /**
         * \brief Port on which the socket is listening.
         */
        uint16_t port_;
    };
} // namespace http
