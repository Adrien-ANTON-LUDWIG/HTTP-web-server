#include "sendhealthcheck.hh"

namespace http
{
    SendHealthCheckEW::SendHealthCheckEW(std::shared_ptr<Host> host,
                                         const shared_socket &backend_sock,
                                         const shared_connection &connection)
        : EventWatcher(connection->sock->fd_get()->fd_, EV_WRITE)
        , host_(host)
        , backend_sock_(backend_sock)
        , connection_(connection)
    {
        request_ = "";
        request_ += "HEAD " + host->health + " HTTP/1.1\r\n";

        if (backend_sock->is_ipv6())
            request_ += "Host: [" + host->ip + "]:" + std::to_string(host->port)
                + "\r\n";
        else
            request_ +=
                "Host: " + host->ip + ":" + std::to_string(host->port) + "\r\n";

        request_ += "Connection: close";
        request_ += "\r\n\r\n";
    }

    void SendHealthCheckEW::operator()()
    {
        try
        {
            char buffer[BUFFER_SIZE];
            size_t copied = request_.copy(buffer, BUFFER_SIZE, 0);
            if (!copied)
            {
                event_register.register_event<RecvHealthCheckEW>(
                    host_, connection_, backend_sock_);
                event_register.unregister_ew(this);
                return;
            }
            auto sent = backend_sock_->send(buffer, copied);
            request_.erase(request_.begin(), request_.begin() + sent);
            if (sent <= 0)
            {
                event_register.register_event<RecvHealthCheckEW>(
                    host_, connection_, backend_sock_);
                event_register.unregister_ew(this);
            }
        }
        catch (const std::exception &e)
        {
#ifdef _DEBUG
            std::cerr << "Could not send the health check to the backend:\n";
            std::cerr << e.what() << std::endl;
#endif
            event_register.unregister_ew(this);
        }
    }
} // namespace http*/