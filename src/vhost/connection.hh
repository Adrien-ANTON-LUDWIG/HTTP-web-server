/**
 * \file vhost/connection.hh
 * \brief Connection declaration.
 */

#pragma once

#include <memory>

#include "request/types.hh"
#include "socket/socket.hh"

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
        Connection(const shared_socket &socket)
            : sock_(socket)
        {}
        Connection(const Connection &con) = default;
        Connection &operator=(const Connection &) = default;
        Connection(Connection &&) = default;
        Connection &operator=(Connection &&) = default;
        ~Connection() = default;

        /* FIXME: Add members to store the information relative to the
        ** connection.
        */

        /**
         * \brief Client socket
         */
        shared_socket sock_;

        /**
         * \brief Message of the client
         */
        std::string message = "";
    };

    using shared_connection = std::shared_ptr<Connection>;
} // namespace http
