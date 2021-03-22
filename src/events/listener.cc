#include "listener.hh"

namespace http
{
    void ListenerEW::operator()()
    {
        try
        {
            auto client_socket = sock_->accept(nullptr, nullptr);
            if (client_socket == nullptr)
                return;
            shared_connection connection =
                std::make_shared<Connection>(client_socket, ip_, port_);
            event_register.register_event<RecvHeadersEW>(connection);
#ifdef _DEBUG
            std::cout << "Client connected !\n";
#endif
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error while accepting client: " << e.what() << '\n';
            return;
        }
    }
} // namespace http