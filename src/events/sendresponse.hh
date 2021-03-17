#pragma once

#include <arpa/inet.h>
#include <iostream>
#include <string>

#include "events/events.hh"
#include "events/recvheaders.hh"
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
            , connection_(connection)
            , response_(response)
        {}

        void operator()() final;

    private:
        shared_connection connection_;
        struct Response response_;
        bool sending_body = false;
    };
} // namespace http
