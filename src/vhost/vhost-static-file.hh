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

        bool check_err_response(Request &request,
                                std::shared_ptr<Connection> connection)
        {
            if (!request.is_good())
            {
                event_register.register_event<SendResponseEW>(
                    connection, Response(request.status_code));
                return true;
            }
            return false;
        }

        void respond(Request &request,
                     std::shared_ptr<Connection> connection) final
        {
            if (check_err_response(request, connection))
                return;

            request.uri = conf_.root + request.uri;

            try
            {
                if (std::filesystem::is_directory(request.uri))
                {
                    if (request.uri[request.uri.size() - 1] != '/')
                        request.uri += "/";
                    request.uri += conf_.default_file;
                }
                else if (request.uri[request.uri.size() - 1] == '/')
                    request.uri.erase(request.uri.end() - 1);
            }
            catch (const std::exception &e)
            {
                request.status_code = STATUS_CODE::FORBIDDEN;
                request.content_length = 0;
                request.body.erase();
            }

            if (check_err_response(request, connection))
                return;

            struct Response response(request, request.status_code);
            event_register.register_event<SendResponseEW>(connection, response);
        }
    };
} // namespace http
