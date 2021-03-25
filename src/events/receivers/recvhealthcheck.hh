#pragma once

#include <arpa/inet.h>
#include <iostream>

#include "config/config.hh"
#include "events/events.hh"
#include "events/receivers/recvbody.hh"
#include "events/senders/sendresponse.hh"
#include "misc/define.hh"
#include "vhost/dispatcher.hh"

namespace http
{
    class RecvHealthCheckEW : public EventWatcher
    {
    public:
        /**
         * @brief
         *
         */
        RecvHealthCheckEW(const shared_connection &connection,
                          const shared_socket &backend_sock)
            : EventWatcher(backend_sock->fd_get()->fd_, EV_READ)
            , connection_(connection)
            , backend_sock_(backend_sock)
        {}

        /**
         * \brief Start or resume receiving data from the corresponding client.
         */
        void operator()() final;
        bool check_response();

    private:
        /**
         * @brief Structure connection
         *
         */
        shared_connection connection_;

        shared_socket backend_sock_;

        std::string data;

        struct Host *host_ = nullptr;
    };
} // namespace http
