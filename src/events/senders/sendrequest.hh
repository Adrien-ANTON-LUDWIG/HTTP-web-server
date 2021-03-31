#pragma once

#include <arpa/inet.h>
#include <iostream>
#include <memory>
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
    class SendRequestEW : public EventWatcher
    {
    public:
        explicit SendRequestEW(const std::shared_ptr<Request> &request,
                               const shared_socket &backend_sock,
                               const shared_connection &connection);

        void unregister_proxy_timeout();

        void operator()() final;

    private:
        shared_socket backend_sock_;
        std::string request_string_;
        bool keep_alive = true;
        bool sending_body = false;
    };
} // namespace http
