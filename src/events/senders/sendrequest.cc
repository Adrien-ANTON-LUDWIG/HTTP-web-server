#include "sendrequest.hh"

namespace http
{
    SendRequestEW::SendRequestEW(const std::shared_ptr<Request> &request,
                                 const shared_socket &backend_sock,
                                 const shared_connection &connection)
        : EventWatcher(backend_sock->fd_get()->fd_, EV_WRITE)
        , backend_sock_(backend_sock)
    {
        request_ = request;
        connection_ = connection;
        request_string_ +=
            request->method_string_ + ' ' + request->uri + " HTTP/1.1\r\n";

        for (auto h : request->headers)
        {
            request_string_ += h.first + ": " + h.second + "\r\n";
        }

        request_string_ += "Connection: close";

        request_string_ += "\r\n\r\n";

        if (request->body != "")
            request_string_ += request->body + "\r\n";

        if (connection_->vhost_conf.proxy_pass->timeout.has_value())
        {
            connection_->timeout_proxy = std::make_shared<TimeoutProxy>(
                this, *connection_->vhost_conf.proxy_pass->timeout);
        }
    }

    void SendRequestEW::unregister_proxy_timeout()
    {
        if (connection_->timeout_proxy != nullptr)
        {
            event_register.get_loop().unregister_timer_watcher(
                connection_->timeout_proxy->get_et().get());
        }
    }

    void SendRequestEW::operator()()
    {
        try
        {
            char buffer[BUFFER_SIZE];
            size_t copied = request_string_.copy(buffer, BUFFER_SIZE, 0);
            if (!copied) // TODO: Check is the connection needs to stay alive
            {
                event_register.register_event<RecvResponseEW>(connection_,
                                                              backend_sock_);
                event_register.unregister_ew(this);
                return;
            }
            ssize_t sent = backend_sock_->send(buffer, copied);
            request_string_.erase(request_string_.begin(),
                                  request_string_.begin() + sent);
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