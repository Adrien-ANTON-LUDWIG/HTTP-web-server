#pragma once

#include <arpa/inet.h>
#include <iostream>

#include "events/events.hh"
#include "events/receivers/recvbody.hh"
#include "events/senders/sendresponse.hh"
#include "misc/define.hh"
#include "vhost/dispatcher.hh"

namespace http
{
    /**
     * \class RecvResponseEW
     * \brief Workflow for Receive Headers events.
     */
    class RecvResponseEW : public EventWatcher
    {
    public:
        /**
         * @brief
         *
         */
        explicit RecvResponseEW(const shared_connection &connection,
                                const shared_socket &backend_sock)
            : EventWatcher(backend_sock->fd_get()->fd_, EV_READ)
            , backend_sock_(backend_sock)
        {
            connection_ = connection;
            if (connection->timeout_proxy != nullptr)
                connection->timeout_proxy->set_ew(this);
        }

        void unregister_proxy_timeout();

        void recv_headers();
        void recv_body();
        std::string build_response();

        /**
         * \brief Start or resume receiving data from the corresponding client.
         */
        void operator()() final;

    private:
        /**
         * @brief Structure connection
         *
         */
        shared_socket backend_sock_;

        /**
         * @brief Structure to contain the parsed response.
         *
         */
        std::string response_;
        bool recv_finished = false;
        int content_length_ = -1;
        std::string body_;
        int current_length_ = -2;
    };

} // namespace http
