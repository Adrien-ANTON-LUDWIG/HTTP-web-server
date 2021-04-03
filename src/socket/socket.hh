/**
 * \file socket/socket.hh
 * \brief Socket declaration.
 */

#pragma once

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <memory>
#include <sys/socket.h>

#include "misc/fd.hh"

namespace http
{
    /**
     * \struct Socket
     * \brief Value object representing a socket.
     *
     * socket(7)
     */
    struct Socket
    {
        /**
         * \brief Create a Socket from a fd.
         */
        explicit Socket(const misc::shared_fd &fd)
            : fd_{ fd }
        {
            struct sockaddr_in addr;
            socklen_t addr_size = sizeof(struct sockaddr_in);
            getpeername(fd->fd_, (struct sockaddr *)&addr, &addr_size);

            if (addr.sin_family == AF_INET)
            {
                char buf[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(addr.sin_addr), buf, INET_ADDRSTRLEN);
                hostname_ = buf;
            }
            else
            {
                char buf[INET6_ADDRSTRLEN];
                struct sockaddr_in6 addr6;
                socklen_t addr6_size = sizeof(struct sockaddr_in6);
                getpeername(fd->fd_, (struct sockaddr *)&addr6, &addr6_size);
                inet_ntop(AF_INET6, &(addr6.sin6_addr), buf, INET6_ADDRSTRLEN);
                hostname_ = buf;
            }
        }

        Socket() = default;
        Socket(const Socket &) = delete;
        Socket &operator=(const Socket &) = delete;
        Socket(Socket &&) = default;
        Socket &operator=(Socket &&) = default;
        virtual ~Socket() = default;

        /**
         * \brief recv(2).
         */
        virtual ssize_t recv(void *dst, size_t len) = 0;

        /**
         * \brief send(2).
         */
        virtual ssize_t send(const void *src, size_t len) = 0;

        /**
         * \brief sendfile(2).
         */
        virtual ssize_t sendfile(misc::shared_fd &, off_t &, size_t) = 0;

        /**
         * \brief bind(2).
         */
        virtual void bind(const sockaddr *addr, socklen_t addrlen) = 0;

        /**
         * \brief listen(2).
         */
        virtual void listen(int backlog) = 0;

        /**
         * \brief setsockopt(2).
         */
        virtual void setsockopt(int level, int optname, int optval) = 0;

        /**
         * \brief getsockopt(2).
         */
        virtual void getsockopt(int level, int optname, int &optval) = 0;

        /**
         * \brief accept(2).
         */
        virtual std::shared_ptr<Socket> accept(sockaddr *addr,
                                               socklen_t *addrlen) = 0;

        /**
         * \brief connect(2).
         */
        virtual void connect(const sockaddr *, socklen_t) = 0;

        const misc::shared_fd fd_get() const noexcept
        {
            return fd_;
        }

        bool is_ipv6() const noexcept
        {
            return ipv6_;
        }

        void ipv6_set(bool ipv6) noexcept
        {
            ipv6_ = ipv6;
        }

        std::string get_hostname()
        {
            return hostname_;
        }

        void set_hostname(std::string name)
        {
            hostname_ = name;
        }

    protected:
        /**
         * \brief File descriptor of the socket.
         */
        misc::shared_fd fd_;

        std::string hostname_;
        /**
         * Either ipv4 or ipv6.
         */
        bool ipv6_ = false;
    };

    using shared_socket = std::shared_ptr<Socket>;
} // namespace http
