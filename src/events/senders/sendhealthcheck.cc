#include "sendhealthcheck.hh"

/*namespace http
{
    SendHealthCheckEW::SendHealthCheckEW(const std::string &health_file,
                                         const shared_socket &backend_sock,
                                         const shared_connection &connection)
        : EventWatcher(backend_sock->fd_get()->fd_, EV_WRITE)
        , backend_sock_(backend_sock)
        , connection_(connection)
    {
        request_ += "HEAD" + ' ' + health_file + " HTTP/1.1\r\n";
        request_ += "Host: " + ;

        for (auto h : request.headers)
            request_ += h.first + ": " + h.second + "\r\n";

        request_ += "Connection: close";

        request_ += "\r\n\r\n";

        if (request.body != "")
            request_ += request.body + "\r\n";
    }

    void SendHealthCheckEW::operator()()
    {
        try
        {
            char buffer[BUFFER_SIZE];
            size_t copied = request_.copy(buffer, BUFFER_SIZE, 0);
            if (!copied) // TODO: Check is the connection needs to stay alive
            {
                event_register.register_event<RecvResponseEW>(connection_,
                                                              backend_sock_);
                event_register.unregister_ew(this);
                return;
            }
            size_t sent = backend_sock_->send(buffer, copied);
            request_.erase(request_.begin(), request_.begin() + sent);
            if (sent <= 0)
            {
                event_register.register_event<RecvResponseEW>(connection_,
                                                              backend_sock_);
                event_register.unregister_ew(this);
            }
        }
        catch (const std::exception &e)
        {
#ifdef _DEBUG
            std::cerr << "Could not send the data to the client:\n";
            std::cerr << e.what() << std::endl;
#endif
            event_register.unregister_ew(this);
        }
    }

} // namespace http*/