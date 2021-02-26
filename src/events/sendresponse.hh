#pragma once

#include <arpa/inet.h>
#include <iostream>
#include <string>

#include "events/events.hh"
#include "events/register.hh"
#include "misc/sys-wrapper.hh"
#include "request/response.hh"
#include "socket/socket.hh"

namespace http
{
#define BUFFER_SIZE 4096

    class SendResponseEW : public EventWatcher
    {
    public:
        explicit SendResponseEW(const shared_connection &connection,
                                const Response &response)
            : EventWatcher(connection->sock->fd_get()->fd_, EV_WRITE)
            , connection_(connection)
            , response_(response)
        {}

        void operator()() final
        {
            try
            {
                if (!sending_body)
                {
                    char buffer[BUFFER_SIZE];
                    auto len = response_.response.copy(buffer, BUFFER_SIZE, 0);
                    if (connection_->sock->send(buffer, len) <= 0)
                        throw std::ifstream::failure(
                            "Connection closed (header)");
                    response_.response.erase(response_.response.begin(),
                                             response_.response.begin() + len);
#ifdef _DEBUG
                    std::cout << std::string(buffer, len) << '\n';
#endif
                }
                if (!response_.response.size())
                {
                    sending_body = true;
                    if (!response_.file_stream.is_open())
                    {
                        event_register.unregister_ew(this);
                        return;
                    }
                    char buffer[BUFFER_SIZE];
                    auto len =
                        response_.file_stream.readsome(buffer, BUFFER_SIZE);
                    auto sent = connection_->sock->send(buffer, len);
                    if (sent <= 0)
                        throw std::ifstream::failure(
                            "Connection closed (body)");
                    response_.file_stream.seekg(sent - len, std::ios_base::cur);
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Could not send the data to the client:\n";
                std::cerr << e.what() << std::endl;
                event_register.unregister_ew(this);
                return;
            }
        }

    private:
        shared_connection connection_;
        struct Response response_;
        bool sending_body = false;
    };
} // namespace http
