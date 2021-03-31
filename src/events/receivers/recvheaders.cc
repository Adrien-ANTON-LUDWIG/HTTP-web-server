#include "recvheaders.hh"

namespace http
{
    void RecvHeadersEW::handle_timeout_begin()
    {
        if (connection_->timeout_keep_alive.has_value())
        {
            auto et = connection_->timeout_keep_alive.value().get_et().get();
            event_register.get_loop().unregister_timer_watcher(et);
            connection_->timeout_keep_alive.reset();
        }
        if (http::dispatcher.serv_config_.timeout_transaction.has_value())
        {
            connection_->timeout_transaction = Timeout(
                this, *http::dispatcher.serv_config_.timeout_transaction,
                Timeout::transaction_cb);
        }
        if (http::dispatcher.serv_config_.timeout_throughput_time.has_value())
        {
            connection_->timeout_throughput = Timeout(
                this, *http::dispatcher.serv_config_.timeout_throughput_time,
                *http::dispatcher.serv_config_.timeout_throughput_val,
                Timeout::throughput_cb);
        }
    }

    void RecvHeadersEW::unregister_timeout()
    {
        if (connection_->timeout_transaction.has_value())
        {
            event_register.get_loop().unregister_timer_watcher(
                connection_->timeout_transaction->get_et().get());
            connection_->timeout_transaction.reset();
        }
        if (connection_->timeout_throughput.has_value())
        {
            event_register.get_loop().unregister_timer_watcher(
                connection_->timeout_throughput->get_et().get());
            connection_->timeout_throughput.reset();
        }
    }

    void RecvHeadersEW::operator()()
    {
        handle_timeout_begin();

        char buffer[BUFFER_SIZE];
        auto read_size = connection_->sock->recv(buffer, BUFFER_SIZE);

        if (read_size == -1)
        {
            unregister_timeout();
            event_register.unregister_ew(this);
            return;
        }

        if (connection_->timeout_throughput.has_value())
            connection_->timeout_throughput->received_bytes(read_size);
        connection_->message.append(buffer, buffer + read_size);
        std::string carriage = "\r\n\r\n";

        if (connection_->message.find(carriage) != std::string::npos)
        {
#ifdef _DEBUG
            std::cout << "RecvHeadersEW request :\n"
                      << connection_->message << "\n";
#endif
            Request req;
            req.parse_request(connection_->message);
            request_ = std::make_shared<Request>(req);

            connection_->keep_alive = request_->keep_alive;

            request_->pretty_print();
            if (request_->method == Method::POST
                && request_->content_length != 0
                && request_->body.size() < request_->content_length)
                event_register.register_event<RecvBodyEW>(connection_,
                                                          request_);
            else
                dispatcher.dispatch(connection_, request_);

            unregister_timeout();
            event_register.unregister_ew(this);
            connection_->message.erase();
        }
        else if (read_size <= 0)
        {
            unregister_timeout();
            event_register.unregister_ew(this);
        }
    }

} // namespace http
