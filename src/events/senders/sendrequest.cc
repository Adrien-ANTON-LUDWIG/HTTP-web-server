#include "sendrequest.hh"

namespace http
{
    void SendRequestEW::operator()()
    {
        try
        {
            /*
            if (response_.response.size())
            {
                char buffer[BUFFER_SIZE];
                auto len = response_.response.copy(buffer, BUFFER_SIZE, 0);
                if (connection_->sock->send(buffer, len) <= 0)
                    throw std::ifstream::failure("Connection closed (header)");
                response_.response.erase(response_.response.begin(),
                                         response_.response.begin() + len);
#ifdef _DEBUG
                std::cout << std::string(buffer, len) << '\n';
#endif
            }
            else
            {
                if (!response_.file_stream.is_open())
                {
                    event_register.unregister_ew(this);
                    return;
                }
                char buffer[BUFFER_SIZE];
                auto len = response_.file_stream.readsome(buffer, BUFFER_SIZE);
                if (len <= 0)
                {
                    if (connection_->keep_alive)
                        event_register.register_event<RecvHeadersEW>(
                            connection_);
                    event_register.unregister_ew(this);
                    return;
                }
                auto sent = connection_->sock->send(buffer, len);
                if (sent <= 0)
                    throw std::ifstream::failure("Connection closed (body)");
                response_.file_stream.seekg(sent - len, std::ios_base::cur);
            }
            */
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