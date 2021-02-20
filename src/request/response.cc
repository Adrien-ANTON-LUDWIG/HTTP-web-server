#include "request/response.hh"

#include <ctime>
#include <fstream>
#include <istream>
#include <sstream>

#include "request/types.hh"

namespace http
{
    Response::Response(const STATUS_CODE &)
    {
        /* FIXME */
    }

    Response::Response(const Request &req, const STATUS_CODE &code)
    {
        char buffer[512];
        auto realcode = code;

        std::ifstream file;
        std::string content;

        try
        {
            file.open(req.uri);
            ssize_t r = 0;
            while ((r = file.readsome(buffer, 512)) > 0)
                content += std::string(buffer, r);
            file.close();
            realcode = STATUS_CODE::OK;
        }
        catch (const std::exception &e)
        {
            content = "";
            realcode = STATUS_CODE::NOT_FOUND;
            std::cerr << e.what() << '\n';
        }

        if (req.method == Method::HEAD)
            content = "";

        auto codepair = statusCode(realcode);
        char datebuffer[256];

        response = "HTTP/1.1 ";
        std::to_string(codepair.first) + " " + codepair.second + "\r\n";

        time_t now;
        time(&now);
        auto now_time = gmtime(&now);

        size_t time_size =
            strftime(datebuffer, 256, "%a, %d %b %Y %X %Z\r\n", now_time);
        response += "Date: " + std::string(datebuffer, time_size);
        response +=
            "Content-Length: " + std::to_string(content.size()) + "\r\n";
        response += "Connection: close\r\n";
        response += "Server: Spider - Pierre da god Edition\r\n";
        response += "\r\n";
        response += content;
    }
} // namespace http
