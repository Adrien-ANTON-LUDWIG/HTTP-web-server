/**
 * \file vhost/vhost-static-file.hh
 * \brief VHostStaticFile declaration.
 */

#pragma once

#include <filesystem>

#include "config/config.hh"
#include "events/sendresponse.hh"
#include "request/request.hh"
#include "request/response.hh"
#include "vhost/connection.hh"
#include "vhost/vhost.hh"

namespace http
{
    /**
     * \class VHostStaticFile
     * \brief VHost serving static files.
     */
    class VHostStaticFile : public VHost
    {
    public:
        friend class VHostFactory;
        virtual ~VHostStaticFile() = default;

    private:
        /**
         * \brief Constructor called by the factory.
         *
         * \param config VHostConfig virtual host configuration.
         */
        explicit VHostStaticFile(const VHostConfig &vhost)
            : VHost(vhost)
        {}

    public:
        /**
         * \brief Send response.
         *
         * \param req Request.
         * \param conn Connection.
         *
         * Note that these iterators will only be useful starting from SRPS.
         */
        void respond(Request &request,
                     std::shared_ptr<Connection> connection) final
        {
            request.uri = conf_.root + request.uri;

            if (std::filesystem::is_directory(request.uri))
                request.uri += conf_.default_file;

            if (request.status_code != STATUS_CODE::OK)
            {
                event_register.register_event<SendResponseEW>(
                    connection, Response(request.status_code));
                return;
            }

            struct Response response(request, request.status_code);
            event_register.register_event<SendResponseEW>(connection, response);
        }
    };
} // namespace http
