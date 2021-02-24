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
#define BUFFER_SIZE 512

    class SendResponseEW : public EventWatcher
    {
    public:
        explicit SendResponseEW(const shared_connection &connection,
                                const Response &response)
            : EventWatcher(connection->sock_->fd_get()->fd_, EV_WRITE)
            , connection_(connection)
            , response_(response)
        {}

        void operator()() final
        {
            try
            {
                char buffer[BUFFER_SIZE];
                auto len = response_.response.copy(buffer, BUFFER_SIZE, 0);
                connection_->sock_->send(buffer, len);

#ifdef _DEBUG
                std::cout << std::string(buffer, len) << '\n';
#endif

                response_.response.erase(response_.response.begin(),
                                         response_.response.begin() + len);
                if (!response_.response.size())
                {
                    event_register.unregister_ew(this);
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Could not send the data to the client:\n";
                std::cerr << e.what();
                event_register.unregister_ew(this);
                return;
            }
        }

    private:
        shared_connection connection_;
        struct Response response_;
    };
} // namespace http
