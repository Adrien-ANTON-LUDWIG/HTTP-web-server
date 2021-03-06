#include "socket/ssl-socket.hh"

#include <iostream>
#include <openssl/ssl.h>

#include "misc/fd.hh"
#include "misc/openssl/ssl.hh"
#include "misc/socket.hh"

namespace http
{
    SSLSocket::SSLSocket(int domain, int type, int protocol, SSL_CTX *ssl_ctx)
        : Socket{ std::make_shared<misc::FileDescriptor>(
            sys::socket(domain, type, protocol)) }
        , ssl_(SSL_new(ssl_ctx), SSL_free)
    {}

    SSLSocket::SSLSocket(const misc::shared_fd &fd, SSL_CTX *ssl_ctx)
        : Socket(fd)
        , ssl_(SSL_new(ssl_ctx), SSL_free)
    {
        SSL_set_fd(ssl_.get(), fd->fd_);
        ssl::accept(ssl_.get());
    }

    ssize_t SSLSocket::recv(void *dst, size_t len)
    {
        try
        {
            return ssl::read(ssl_.get(), dst, len);
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
            return ssl::write(ssl_.get(), buf, buf_len);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Send Failed : << " << e.what() << '\n';
            return -1;
        }
    }

    ssize_t SSLSocket::sendfile(misc::shared_fd &, off_t &, size_t)
    {
        return -1;
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
        try
        {
            misc::shared_fd fd_ptr = std::make_shared<misc::FileDescriptor>(
                sys::accept(*fd_, addr, addrlen));
            shared_socket ptr = std::make_shared<SSLSocket>(
                fd_ptr, SSL_get_SSL_CTX(ssl_.get()));

            sys::fcntl_set(*ptr->fd_get(), O_NONBLOCK);
            return ptr;
        }
        catch (const std::exception &e)
        {
            std::cerr << "SSL accept: " << e.what() << '\n';
            return nullptr;
        }
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
