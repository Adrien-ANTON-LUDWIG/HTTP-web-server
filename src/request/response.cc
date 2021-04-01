#include "request/response.hh"

#include <ctime>
#include <filesystem>
#include <fstream>
#include <istream>
#include <sstream>

#include "misc/define.hh"
#include "request/types.hh"

namespace http
{
    void Response::build_err_response(const STATUS_CODE &code)
    {
        auto codepair = statusCode(code);
        char datebuffer[BUFFER_SIZE];
        response = "HTTP/1.1 " + std::to_string(codepair.first) + " "
            + codepair.second + "\r\n";

        time_t now;
        time(&now);
        auto now_time = gmtime(&now);

        size_t time_size = strftime(datebuffer, BUFFER_SIZE,
                                    "%a, %d %b %Y %X %Z\r\n", now_time);

        response += "Date: " + std::string(datebuffer, time_size);
        response += "Content-Length: 0\r\n";
        response += "Connection: close\r\n";
    }

    Response::Response(const STATUS_CODE &code)
    {
        build_err_response(code);
        response += "\r\n";
    }

    Response::Response(const STATUS_CODE &code, const std::string &timeout)
    {
        build_err_response(code);
        response += "X-Timeout-Reason: " + timeout;
        response += "\r\n\r\n";
    }

    Response::Response(const Request &req, const STATUS_CODE &code)
    {
        auto realcode = code;
        auto file_size = 0;

        std::string body;
        if (code == STATUS_CODE::OK)
        {
            file_stream.open(req.uri);
            file_path = req.uri;
            if (!file_stream.is_open())
            {
                if (std::filesystem::exists(req.uri))
                    realcode = STATUS_CODE::FORBIDDEN;
                else
                    realcode = STATUS_CODE::NOT_FOUND;
            }
            try
            {
                file_size = std::filesystem::file_size(req.uri);
            }
            catch (const std::exception &)
            {}
        }

        if (req.method == Method::HEAD)
        {
            body = "";
            file_stream.close();
        }

        auto codepair = statusCode(realcode);
        char datebuffer[BUFFER_SIZE];

        response = "HTTP/1.1 " + std::to_string(codepair.first) + " "
            + codepair.second + "\r\n";

        if (code == STATUS_CODE::UNAUTHORIZED
            || code == STATUS_CODE::PROXY_AUTHENTICATION_REQUIRED)
        {
            if (code == STATUS_CODE::PROXY_AUTHENTICATION_REQUIRED)
                response += "Proxy-Authenticate: Basic realm=\"";
            else
                response += "WWW-Authenticate: Basic realm=\"";
            response += req.auth_basic + "\"\r\n";
        }

        time_t now;
        time(&now);
        auto now_time = gmtime(&now);

        size_t time_size = strftime(datebuffer, BUFFER_SIZE,
                                    "%a, %d %b %Y %X %Z\r\n", now_time);
        response += "Date: " + std::string(datebuffer, time_size);

        response += "Content-Length: " + std::to_string(file_size) + "\r\n";
        response += "Connection: ";
        response += req.keep_alive ? "keep-alive" : "close";
        response += "\r\n\r\n";
    }

    Response::Response(const Request &req, const STATUS_CODE &code,
                       std::string &list_directory)
    {
        auto realcode = code;

        auto codepair = statusCode(realcode);
        char datebuffer[BUFFER_SIZE];

        response = "HTTP/1.1 " + std::to_string(codepair.first) + " "
            + codepair.second + "\r\n";

        if (code == STATUS_CODE::UNAUTHORIZED
            || code == STATUS_CODE::PROXY_AUTHENTICATION_REQUIRED)
        {
            if (code == STATUS_CODE::PROXY_AUTHENTICATION_REQUIRED)
                response += "Proxy-Authenticate: Basic realm=\"";
            else
                response += "WWW-Authenticate: Basic realm=\"";
            response += req.auth_basic + "\"\r\n";
        }

        time_t now;
        time(&now);
        auto now_time = gmtime(&now);

        size_t time_size = strftime(datebuffer, BUFFER_SIZE,
                                    "%a, %d %b %Y %X %Z\r\n", now_time);
        response += "Date: " + std::string(datebuffer, time_size);

        response += "Content-Length: "
            + (code == STATUS_CODE::OK ? std::to_string(list_directory.size())
                                       : 0)
            + "\r\n";
        response += "Connection: ";
        response += req.keep_alive ? "keep-alive" : "close";
        response += "\r\n\r\n";
    }

    Response::Response(const std::string &resp)
        : response(resp)
    {}
} // namespace http
