/**
 * \file vhost/connection.hh
 * \brief Connection declaration.
 */

#pragma once

#include <memory>

#include "config/config.hh"
#include "request/types.hh"
#include "socket/socket.hh"
#include "timeout/timeout_keepalive.hh"
#include "timeout/timeout_proxy.hh"
#include "timeout/timeout_throughput.hh"
#include "timeout/timeout_transaction.hh"

namespace http
{
    /**
     * \struct Connection
     * \brief Value object representing a connection.
     *
     * We need to keep track of the state of each request while it has not
     * been fully processed.
     */
    struct Connection
    {
        Connection() = default;
        Connection(const shared_socket &socket, const std::string &ip,
                   const uint &port)
            : sock(socket)
            , listener_ip(ip)
            , listener_port(port)
        {}

        Connection(const Connection &con) = default;
        Connection &operator=(const Connection &) = default;
        Connection(Connection &&) = default;
        Connection &operator=(Connection &&) = default;
        ~Connection() = default;

        /**
         * @brief
         *
         */
        shared_socket sock = nullptr;

        /**
         * @brief Listener information to check while dispatching.
         *
         */
        std::string listener_ip = "";
        uint listener_port = 0;

        VHostConfig vhost_conf;

        bool keep_alive = true;

        /**
         * \brief Message of the client
         */
        std::string message = "";
        std::shared_ptr<TimeoutKeepAlive> timeout_keep_alive = nullptr;
        std::shared_ptr<TimeoutTransaction> timeout_transaction = nullptr;
        std::shared_ptr<TimeoutThroughput> timeout_throughput = nullptr;
        std::shared_ptr<TimeoutProxy> timeout_proxy = nullptr;
    };
    using shared_connection = std::shared_ptr<Connection>;
} // namespace http
