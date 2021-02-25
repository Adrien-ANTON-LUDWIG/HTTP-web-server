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
    Response::Response(const STATUS_CODE &code)
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

    Response::Response(const Request &req, const STATUS_CODE &code)
    {
        char buffer[BUFFER_SIZE];
        auto realcode = code;

        std::ifstream file;
        std::string body;

        file.open(req.uri);
        if (!file.is_open())
        {
            if (std::filesystem::exists(req.uri))
                realcode = STATUS_CODE::FORBIDDEN;
            else
                realcode = STATUS_CODE::NOT_FOUND;
        }
        ssize_t r = 0;
        while ((r = file.readsome(buffer, BUFFER_SIZE)) > 0)
            body += std::string(buffer, r);
        file.close();
        if (req.method == Method::HEAD)
            body = "";

        auto codepair = statusCode(realcode);
        char datebuffer[BUFFER_SIZE];

        response = "HTTP/1.1 " + std::to_string(codepair.first) + " "
            + codepair.second + "\r\n";

        time_t now;
        time(&now);
        auto now_time = gmtime(&now);

        size_t time_size = strftime(datebuffer, BUFFER_SIZE,
                                    "%a, %d %b %Y %X %Z\r\n", now_time);
        response += "Date: " + std::string(datebuffer, time_size);

        response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += body;
    }
} // namespace http
