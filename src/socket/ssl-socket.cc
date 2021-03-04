#include "socket/ssl-socket.hh"

#include <iostream>

#include "misc/fd.hh"
#include "misc/socket.hh"

namespace http
{
    SSLSocket::SSLSocket(int domain, int type, int protocol, SSL_CTX *ssl_ctx)
        : Socket{ std::make_shared<misc::FileDescriptor>(
            sys::socket(domain, type, protocol)) }
    {
        (void)ssl_ctx;
    }

    ssize_t SSLSocket::recv(void *dst, size_t len)
    {
        try
        {
            return sys::recv(*fd_, dst, len, 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Reception failed : " << e.what() << '\n';
            return -1;
        }
    }

    ssize_t SSLSocket::send(const void *buf, size_t buf_len)
    {
        try
        {
            return sys::send(*fd_, buf, buf_len, MSG_NOSIGNAL);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Send Failed : << " << e.what() << '\n';
            return -1;
        }
    }

    ssize_t SSLSocket::sendfile(misc::shared_fd &fd, off_t &offset, size_t len)
    {
        try
        {
            return sys::sendfile(*fd_, *fd, &offset, len);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Sendfile Failed : " << e.what() << '\n';
            return -1;
        }
    }

    void SSLSocket::bind(const sockaddr *addr, socklen_t len)
    {
        try
        {
            sys::bind(*fd_, addr, len);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Bind Failed : " << e.what() << '\n';
            throw e;
        }
    }

    void SSLSocket::listen(int backlog)
    {
        try
        {
            sys::listen(*fd_, backlog);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            throw e;
        }
    }

    void SSLSocket::setsockopt(int level, int optname, int optval)
    {
        try
        {
            sys::setsockopt(*fd_, level, optname, &optval, sizeof(int));
        }
        catch (const std::exception &e)
        {
            std::cerr << "Setsockopt Failed: " << e.what() << '\n';
        }
    }

    void SSLSocket::getsockopt(int level, int optname, int &optval)
    {
        try
        {
            unsigned int len = sizeof(int);
            sys::getsockopt(*fd_, level, optname, &optval, &len);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Getsockopt Failed: " << e.what() << '\n';
        }
    }

    shared_socket SSLSocket::accept(sockaddr *addr, socklen_t *addrlen)
    {
        misc::shared_fd fd_ptr = std::make_shared<misc::FileDescriptor>(
            sys::accept(*fd_, addr, addrlen));
        shared_socket ptr = std::make_shared<SSLSocket>(fd_ptr, nullptr);
        sys::fcntl_set(*ptr->fd_get(), O_NONBLOCK);
        return ptr;
    }

    void SSLSocket::connect(const sockaddr *addr, socklen_t addrlen)
    {
        try
        {
            sys::connect(*fd_, addr, addrlen);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Connection Failed: " << e.what() << '\n';
        }
    }

} // namespace http
