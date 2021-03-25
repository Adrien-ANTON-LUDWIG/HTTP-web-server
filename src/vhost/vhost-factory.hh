#pragma once
/**
 * \file vhost/vhost-factory.hh
 * \brief VHostFactory
 */

#include <chrono>
#include <memory>

#include "vhost/vhost-reverse-proxy.hh"
#include "vhost/vhost-static-file.hh"
#include "vhost/vhost.hh"

namespace http
{
    /**
     * \class VHostFactory
     * \brief Factory design pattern to create VHost.
     */
    class VHostFactory
    {
    public:
        /**
         * \brief Create a VHost object from a given VHostConfig.
         */
        static shared_vhost Create(const VHostConfig &vhost_c,
                                   const ServerConfig &server_c)
        {
            if (vhost_c.proxy_pass != std::nullopt)
            {
                if (!vhost_c.proxy_pass->upstream.empty())
                {
                    for (auto &backend : server_c.upstreams)
                    {
                        if (backend.name == vhost_c.proxy_pass->upstream)
                        {
                            // REGISTER EV_TIMER CALLBACK

                            std::shared_ptr<VHost> ptr = std::shared_ptr<VHost>(
                                new VHostReverseProxy(vhost_c, backend));

                            // CREATE AN EVENTWATCHER HEALTH CHECK AVEC LE
                            // BACKEND
                        }
                    }
                    std::cerr << "No backend found\n";
                    exit(1);
                }
                return std::shared_ptr<VHost>(
                    new VHostReverseProxy(vhost_c, std::nullopt));
            }
            return std::shared_ptr<VHost>(new VHostStaticFile(vhost_c));
        }
    };

} // namespace http
