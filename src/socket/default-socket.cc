#include "socket/default-socket.hh"

#include <iostream>

#include "misc/socket.hh"

namespace http
{
    DefaultSocket::DefaultSocket(int domain, int type, int protocol)
        : Socket{ std::make_shared<misc::FileDescriptor>(
            sys::socket(domain, type, protocol)) }
    {}

    ssize_t DefaultSocket::recv(void *dst, size_t len)
    {
        try
        {
            return sys::recv(dst, len);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Reception failed : " << e.what() << '\n';
            return -1;
        }
    }

    ssize_t DefaultSocket::send(const void *buf, size_t buf_len)
    {
        try
        {
            return sys::send(fd_, buf, buf_len, 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Send Failed : << " << e.what() << '\n';
            return -1;
        }
    }

    ssize_t DefaultSocket::sendfile(misc::shared_fd &fd, off_t &offset,
                                    size_t len)
    {
        try
        {
            return sys::sendfile(fd, offset, len);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Sendfile Failed : " << e.what() << '\n';
            return -1;
        }
    }

    void DefaultSocket::bind(const sockaddr *addr, socklen_t len)
    {
        try
        {
            sys::bind(fd_, addr, len);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Bind Failed : " << e.what() << '\n';
            throw e;
        }
    }

    void DefaultSocket::listen(int backlog)
    {
        try
        {
            sys::listen(fd_, backlog);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            throw e;
        }
    }

    void DefaultSocket::setsockopt(int level, int optname, int optval)
    {
        try
        {
            sys::setsockopt(level, optname, optval);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Setsockopt Failed: " << e.what() << '\n';
        }
    }

    void DefaultSocket::getsockopt(int level, int optname, int &optval)
    {
        try
        {
            sys::getsockopt(level, optname, optval);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Getsockopt Failed: " << e.what() << '\n';
        }
    }

    shared_socket DefaultSocket::accept(sockaddr *addr, socklen_t *addrlen)
    {
        try
        {
            auto new_fd = sys::accept(fd_, addr, addrlen);
            return std::make_shared<http::Socket>(new_fd);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Accept Failed : " << e.what() << '\n';
            return nullptr;
        }
    }

    void DefaultSocket::connect(const sockaddr *addr, socklen_t addrlen)
    {
        try
        {
            sys::connect(fd_, addr, addrlen);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Connection Failed: " << e.what() << '\n';
        }
    }

} // namespace http
