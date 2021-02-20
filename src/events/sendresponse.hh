#pragma once

#include <arpa/inet.h>
#include <iostream>
#include <string>

#include "events/events.hh"
#include "events/register.hh"
#include "socket/socket.hh"

namespace http
{
#define BUFFER_SIZE 512

    class SendResponseEW : public EventWatcher
    {
    public:
        explicit SendResponseEW(shared_socket socket)
            : EventWatcher(socket->fd_get()->fd_, EV_WRITE)
        {
            sock_ = socket;
        }

        void operator()() final
        {
            try
            {
                char buffer[BUFFER_SIZE];
                auto len = data_.copy(buffer, BUFFER_SIZE, 0);
                sock_->send(buffer, len);
                data_.erase(data_.begin(), data_.begin() + len);
                if (!data_.size())
                {
                    std::cout << "Response sent\n";
                    event_register.unregister_ew(this);
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Could not send the data to the client:\n";
                std::cerr << e.what();
                return;
            }
        }

    private:
        shared_socket sock_;
        std::string data_;
    };
} // namespace http
