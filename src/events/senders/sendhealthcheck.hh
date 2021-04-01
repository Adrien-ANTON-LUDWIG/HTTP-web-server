#pragma once

#include <arpa/inet.h>
#include <iostream>
#include <string>

#include "events/events.hh"
#include "events/receivers/recvhealthcheck.hh"
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
        explicit SendHealthCheckEW(std::shared_ptr<Host> host,
                                   const shared_socket &backend_sock,
                                   const shared_connection &connection);

        void operator()() final;

    private:
        std::shared_ptr<Host> host_ = nullptr;
        shared_socket backend_sock_;
        shared_connection connection_;
        std::string request_string_;
    };
} // namespace http
