#include "recvresponse.hh"

namespace http
{
    void RecvResponseEW::operator()()
    {
        /*char buffer[BUFFER_SIZE];
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
            response_.parse_response(connection_->message);

            response_.pretty_print();
            if (response_.method == Method::POST
                && response_.content_length != 0
                && response_.body.size() < response_.content_length)
                event_register.register_event<RecvBodyEW>(connection_,
                                                          response_);
            else
                dispatcher.dispatch(connection_, response_);

            event_register.unregister_ew(this);
            connection_->message.erase();
        }
        else if (read_size <= 0)
        {
            event_register.unregister_ew(this);
        }*/
    }
} // namespace http