#include "sendrequest.hh"

namespace http
{
    SendRequestEW::SendRequestEW(const Request &request,
                                 const shared_socket &backend_sock,
                                 const shared_connection &connection)
        : EventWatcher(backend_sock->fd_get()->fd_, EV_WRITE)
        , backend_sock_(backend_sock)
        , connection_(connection)
    {
        std::string methods[4] = { "GET", "HEAD", "POST", "ERR" };
        request_ += methods[static_cast<int>(request.method)] + ' '
            + request.uri + " HTTP/1.1\r\n";

        for (auto h : request.headers)
            request_ += h.first + ": " + h.second + "\r\n";

        request_ += "Connection: close";

        request_ += "\r\n\r\n";

        if (request.body != "")
            request_ += request.body + "\r\n";
    }

    void SendRequestEW::operator()()
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
            ssize_t sent = backend_sock_->send(buffer, copied);
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

} // namespace http