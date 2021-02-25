/**
 * \file vhost/dispatcher.hh
 * \brief Dispatcher declaration.
 */

#pragma once

#include "vhost/vhost.hh"

namespace http
{
    /**
     * \class Dispatcher
     * \brief Instance in charge of dispatching requests between vhosts.
     */
    class Dispatcher
    {
        using iterator = std::vector<shared_vhost>::iterator;
        using const_iterator = std::vector<shared_vhost>::const_iterator;

    public:
        Dispatcher() = default;
        Dispatcher(const Dispatcher &) = delete;
        Dispatcher &operator=(const Dispatcher &) = delete;
        Dispatcher(Dispatcher &&) = delete;
        Dispatcher &operator=(Dispatcher &&) = delete;

        void add_vhost(const shared_vhost &vhost)
        {
            vhosts_.push_back(vhost);
        }

        void dispatch(const shared_connection connection,
                      struct Request &request)
        {
            auto v_conf = vhosts_[0]->conf_get();

            if (request.host == "" || connection->listener_ip != v_conf.ip
                || connection->listener_port != v_conf.port)
                request.status_code = STATUS_CODE::BAD_REQUEST;

            auto index = request.host.find_last_of(':');

            if (index != std::string::npos
                && request.host != connection->listener_ip
                && request.host != v_conf.server_name)
            {
                std::string host_or_ip(request.host.begin(),
                                       request.host.begin() + index);
                std::string port(request.host.begin() + index + 1,
                                 request.host.end());

                if (((host_or_ip != v_conf.server_name
                      && host_or_ip != connection->listener_ip)
                     || port != std::to_string(connection->listener_port)))
                    request.status_code = STATUS_CODE::BAD_REQUEST;
            }
            else if (request.host != connection->listener_ip
                     && request.host != v_conf.server_name)
                request.status_code = STATUS_CODE::BAD_REQUEST;

            // FIXME Renvoyer vers le vhost le moins chargé

            vhosts_[0]->respond(request, connection);
        }

        iterator begin()
        {
            return vhosts_.begin();
        }

        iterator end()
        {
            return vhosts_.end();
        }

        const_iterator begin() const
        {
            return vhosts_.cbegin();
        }

        const_iterator end() const
        {
            return vhosts_.cend();
        }

    private:
        /* FIXME: Add members to store the information relative to the
        ** Dispatcher.
        */
        std::vector<shared_vhost> vhosts_;
    };

    /**
     * \brief Service object.
     */
    extern Dispatcher dispatcher;
} // namespace http
