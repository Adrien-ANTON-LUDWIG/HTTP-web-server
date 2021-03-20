#include "vhost-static-file.hh"

namespace http
{
    static bool check_err_response(Request &request,
                                   std::shared_ptr<Connection> connection)
    {
        if (request.is_fatal())
        {
            connection->keep_alive = false;
            event_register.register_event<SendResponseEW>(
                connection, Response(request.status_code));
            return true;
        }
        return false;
    }

    void VHostStaticFile::respond(Request &request,
                                  std::shared_ptr<Connection> connection)
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
        if (request.status_code == STATUS_CODE::UNAUTHORIZED)
        {
            request.content_length = 0;
            request.body.erase();
        }
        struct Response response(request, request.status_code);
        connection->keep_alive = request.keep_alive;
        event_register.register_event<SendResponseEW>(connection, response);
    }

} // namespace http