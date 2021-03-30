#pragma once

#include <arpa/inet.h>
#include <iostream>
#include <string>

#include "events/events.hh"
#include "events/receivers/recvheaders.hh"
#include "events/register.hh"
#include "misc/define.hh"
#include "misc/sys-wrapper.hh"
#include "request/response.hh"
#include "socket/socket.hh"

namespace http
{
    class SendResponseEW : public EventWatcher
    {
    public:
        explicit SendResponseEW(const shared_connection &connection,
                                const Response &response)
            : EventWatcher(connection->sock->fd_get()->fd_, EV_WRITE)
            , response_(response)
        {
            connection_ = connection;
        }

        void operator()() final;

    private:
        struct Response response_;
        bool sending_body = false;
    };
} // namespace http
