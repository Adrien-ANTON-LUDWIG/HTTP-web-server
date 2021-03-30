#include "recvheaders.hh"

namespace http
{
    void RecvHeadersEW::operator()()
    {
        // TODO Unregister timeout keep alive, register timeout Transaction and
        // Throughput
        char buffer[BUFFER_SIZE];
        auto read_size = connection_->sock->recv(buffer, BUFFER_SIZE);

        if (read_size == -1)
        {
            // TODO Unregister timeout Transaction and Throughput
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
            request_->parse_request(connection_->message);
            connection_->keep_alive = request_->keep_alive;

            request_->pretty_print();
            if (request_->method == Method::POST
                && request_->content_length != 0
                && request_->body.size() < request_->content_length)
                event_register.register_event<RecvBodyEW>(connection_,
                                                          request_);
            else
                dispatcher.dispatch(connection_, request_);

            // TODO Unregister timeout Transaction and Throughput
            event_register.unregister_ew(this);
            connection_->message.erase();
        }
        else if (read_size <= 0)
        {
            // TODO Unregister timeout Transaction and Throughput
            event_register.unregister_ew(this);
        }
    }

} // namespace http
