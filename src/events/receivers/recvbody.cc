#include "recvbody.hh"
namespace http
{
    void RecvBodyEW::operator()()
    {
        char buffer[BUFFER_SIZE];
        auto read_size = connection_->sock->recv(buffer, BUFFER_SIZE);
        request_.body += std::string(buffer, read_size);
        request_.current_length += read_size;

        if (request_.content_length <= request_.current_length)
        {
#ifdef _DEBUG
            std::cout << request_.body << std::endl;
#endif
            dispatcher.dispatch(connection_, request_);
            event_register.unregister_ew(this);
        }
        else if (read_size <= 0)
        {
            event_register.unregister_ew(this);
#ifdef _DEBUG
            std::cerr << "Connection closed (receive)\n";
#endif
        }
    }
} // namespace http
