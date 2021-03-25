/**
 * \file vhost/vhost-reverse-proxy.hh
 * \brief VhostReverseProxy declaration.
 */

#pragma once

#include <ev.h>
#include <filesystem>
#include <memory>
#include <optional>

#include "config/config.hh"
#include "events/senders/sendhealthcheck.hh"
#include "events/senders/sendrequest.hh"
#include "misc/addrinfo/addrinfo-iterator.hh"
#include "misc/addrinfo/addrinfo.hh"
#include "request/request.hh"
#include "vhost/connection.hh"
#include "vhost/vhost.hh"

namespace http
{
    /**
     * \class VHostReverseProxy
     * \brief VHost in charge of forwarding the Request to the upstream/backend
     * service.
     */
    class VHostReverseProxy : public VHost
    {
    public:
        friend class VHostFactory;
        virtual ~VHostReverseProxy() = default;

        void build_request(Request &request,
                           std::shared_ptr<Connection> &connection);

        std::optional<Backend> backend;

        static void timeout_cb(struct ev_loop *loop, ev_timer *w, int revents);

        /**
         * \brief Send request to the upstream.
         *
         * \param req Request.
         * \param conn std::shared_ptr<Connection>.
         */
        void respond(Request &request,
                     std::shared_ptr<Connection> connection) final;

    private:
        /**
         * \brief Constructor called by the factory.
         *
         * \param config VHostConfig virtual host configuration.
         */
        explicit VHostReverseProxy(const VHostConfig &vhost,
                                   const std::optional<Backend> &b)
            : VHost(vhost)
            , backend(b)
        {
            ev_timer et_;
            et_.data = &backend;
            ev_timer_init(&et_, timeout_cb, 0., 12.);
            event_register.get_loop().register_timer_watcher(&et_);
        }
    };
} // namespace http
