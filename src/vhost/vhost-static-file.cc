#include "vhost-static-file.hh"

#include <fstream>
#include <iostream>

namespace http
{
    static bool check_err_response(std::shared_ptr<Request> request,
                                   std::shared_ptr<Connection> connection)
    {
        if (request->is_fatal())
        {
            connection->keep_alive = false;
            event_register.register_event<SendResponseEW>(
                connection, Response(request->status_code));
            return true;
        }
        return false;
    }

    static std::string list_directory(std::string &uri_path,
                                      std::string &path_without_root)
    {
        bool add_slash = path_without_root[path_without_root.size() - 1] != '/';
        std::string index = "";
        index += "<!DOCTYPE html>\n<html>\n<head>\n<meta "
                 "charset=utf-8>\n<title>Index of "
            + path_without_root + "</title>\n</head>\n<body>\n<ul>\n";
        index += "<li><a href=\"" + path_without_root + (add_slash ? "/" : "")
            + "..\">..</a></li>\n";
        for (auto &file : std::filesystem::directory_iterator(uri_path))
        {
            index += "<li><a href=\"" + path_without_root
                + (add_slash ? "/" : "") + file.path().stem().string()
                + file.path().extension().stem().string() + "\">"
                + file.path().filename().stem().string()
                + file.path().extension().stem().string() + "</a></li>\n";
        }
        index += "</ul>\n</body>\n</html>";
        return index;
    }

    void VHostStaticFile::respond(std::shared_ptr<Request> request,
                                  std::shared_ptr<Connection> connection)
    {
        if (check_err_response(request, connection))
            return;

        std::string uri_without_root = request->uri;
        request->uri = conf_.root + request->uri;

        try
        {
            if (std::filesystem::is_directory(request->uri)
                || request->uri[request->uri.size() - 1] == '/')
            {
                if (request->uri[request->uri.size() - 1] != '/')
                {
                    request->uri += "/";
                }
                if (!std::filesystem::is_directory(request->uri))
                {
                    request->status_code = STATUS_CODE::NOT_FOUND;
                    request->content_length = 0;
                    request->body.erase();
                }
                else if (conf_.auto_index
                         && !std::filesystem::exists(request->uri
                                                     + conf_.default_file))
                {
                    connection->is_list_directory = true;
                    connection->list_directory =
                        list_directory(request->uri, uri_without_root);
                }
                else
                    request->uri += conf_.default_file;
            }
        }
        catch (const std::exception &e)
        {
            request->status_code = STATUS_CODE::FORBIDDEN;
            request->content_length = 0;
            request->body.erase();
        }

        if (check_err_response(request, connection))
            return;
        if (request->status_code == STATUS_CODE::UNAUTHORIZED)
        {
            request->content_length = 0;
            request->body.erase();
        }
        struct Response response;
        if (connection->is_list_directory)
            response = Response(*request, request->status_code,
                                connection->list_directory);
        else
            response = Response(*request, request->status_code);
        event_register.register_event<SendResponseEW>(connection, response);
    }

} // namespace http