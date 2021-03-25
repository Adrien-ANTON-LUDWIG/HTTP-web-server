#pragma once

#include <arpa/inet.h>
#include <iostream>
#include <string>

#include "events/events.hh"
#include "events/receivers/recvheaders.hh"
#include "events/receivers/recvresponse.hh"
#include "events/register.hh"
#include "misc/addrinfo/addrinfo.hh"
#include "misc/define.hh"
#include "misc/sys-wrapper.hh"
#include "request/response.hh"
#include "socket/default-socket.hh"
#include "socket/socket.hh"

namespace http
{
    class SendHealthCheckEW : public EventWatcher
    {
    public:
        explicit SendHealthCheckEW(const std::string &health_file,
                                   const shared_socket &backend_sock,
                                   const shared_connection &connection);

        void operator()() final;

    private:
        shared_socket backend_sock_;
        shared_connection connection_;
        std::string request_;
        bool keep_alive = true;
        bool sending_body = false;
    };
} // namespace http
