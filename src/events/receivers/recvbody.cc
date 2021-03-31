#include "recvbody.hh"
namespace http
{
    void RecvBodyEW::unregister_timeout()
    {
        if (connection_->timeout_transaction != nullptr)
        {
            event_register.get_loop().unregister_timer_watcher(
                connection_->timeout_transaction->get_et().get());
            connection_->timeout_transaction = nullptr;
        }
        if (connection_->timeout_throughput != nullptr)
        {
            event_register.get_loop().unregister_timer_watcher(
                connection_->timeout_throughput->get_et().get());
            connection_->timeout_throughput = nullptr;
        }
    }

    void RecvBodyEW::operator()()
    {
        char buffer[BUFFER_SIZE];
        auto read_size = connection_->sock->recv(buffer, BUFFER_SIZE);
        request_->body += std::string(buffer, read_size);
        request_->current_length += read_size;

        if (request_->content_length <= request_->current_length)
        {
#ifdef _DEBUG
            std::cout << "RecvBodyEw body :\n"
                      << request_->body << '\n'
                      << std::endl;
#endif
            dispatcher.dispatch(connection_, request_);
            unregister_timeout();
            event_register.unregister_ew(this);
        }
        else if (read_size <= 0)
        {
            unregister_timeout();
            event_register.unregister_ew(this);
#ifdef _DEBUG
            std::cerr << "Connection closed (receive)\n";
#endif
        }
    }
} // namespace http
