/**
 * \file events/recvrequest.hh
 * \brief RecvHeadersEW declaration.
 */

#pragma once

#include <arpa/inet.h>
#include <iostream>

#include "events/events.hh"
#include "events/recvbody.hh"
#include "misc/define.hh"
#include "vhost/dispatcher.hh"

namespace http
{
    /**
     * \class RecvHeadersEW
     * \brief Workflow for Receive Headers events.
     */
    class RecvHeadersEW : public EventWatcher
    {
    public:
        /**
         * @brief
         *
         */
        explicit RecvHeadersEW(const shared_connection &connection)
            : EventWatcher(connection->sock_->fd_get()->fd_, EV_READ)
            , connection_(connection)
            , request_()
        {}

        /**
         * \brief Start or resume receiving data from the corresponding client.
         */
        void operator()() final
        {
            char buffer[BUFFER_SIZE];
            auto read_size = connection_->sock_->recv(buffer, BUFFER_SIZE);
            connection_->message.append(buffer, buffer + read_size);
            std::string carriage = "\r\n\r\n";

            if (connection_->message.find(carriage) != std::string::npos)
            {
#ifdef _DEBUG
                std::cout << connection_->message;
#endif
                request_.parse_headers(connection_->message);

                request_.pretty_print();
                if (request_.method == Method::POST
                    && request_.content_length != 0
                    && request_.body.size() < request_.content_length)
                    event_register.register_event<RecvBodyEW>(connection_,
                                                              request_);
                else
                    dispatcher.dispatch(connection_, request_);

                event_register.unregister_ew(this);
                connection_->message.erase();
            }
        }

    private:
        /**
         * @brief Structure connection
         *
         */
        shared_connection connection_;

        /**
         * @brief Structure to contain the parsed request.
         *
         */
        struct Request request_;
    };

} // namespace http
