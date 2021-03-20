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
    class SendRequestEW : public EventWatcher
    {
    public:
        explicit SendRequestEW(const shared_connection &connection,
                               const Request &request)
            : EventWatcher(connection->sock->fd_get()->fd_, EV_WRITE)
            , connection_(connection)
            , request_(request)
        {}

        void operator()() final;

    private:
        shared_connection connection_;
        struct Request request_;
        bool sending_body = false;
    };
} // namespace http
