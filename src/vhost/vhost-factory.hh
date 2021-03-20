#pragma once
/**
 * \file vhost/vhost-factory.hh
 * \brief VHostFactory
 */

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
        static shared_vhost Create(const VHostConfig &vhost_c)
        {
            if (vhost_c.proxy_pass != std::nullopt)
                return std::shared_ptr<VHost>(new VHostReverseProxy(vhost_c));
            return std::shared_ptr<VHost>(new VHostStaticFile(vhost_c));
        }
    };

} // namespace http
