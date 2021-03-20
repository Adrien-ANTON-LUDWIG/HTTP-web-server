#include "recvheaders.hh"

namespace http
{
    void RecvHeadersEW::operator()()
    {
        char buffer[BUFFER_SIZE];
        auto read_size = connection_->sock->recv(buffer, BUFFER_SIZE);

        if (read_size == -1)
        {
            event_register.unregister_ew(this);
            return;
        }

        connection_->message.append(buffer, buffer + read_size);
        std::string carriage = "\r\n\r\n";

        if (connection_->message.find(carriage) != std::string::npos)
        {
#ifdef _DEBUG
            std::cout << connection_->message;
#endif
            request_.parse_request(connection_->message);

            request_.pretty_print();
            if (request_.method == Method::POST && request_.content_length != 0
                && request_.body.size() < request_.content_length)
                event_register.register_event<RecvBodyEW>(connection_,
                                                          request_);
            else
                dispatcher.dispatch(connection_, request_);

            event_register.unregister_ew(this);
            connection_->message.erase();
        }
        else if (read_size <= 0)
        {
            event_register.unregister_ew(this);
        }
    }
} // namespace http
