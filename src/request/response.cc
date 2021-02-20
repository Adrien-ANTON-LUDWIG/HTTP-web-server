#include "request/response.hh"

#include <ctime>
#include <fstream>
#include <istream>
#include <sstream>

#include "misc/define.hh"
#include "request/types.hh"

namespace http
{
    Response::Response(const STATUS_CODE &)
    {
        return;
    }

    Response::Response(const Request &req, const STATUS_CODE &code)
    {
        char buffer[BUFFER_SIZE];
        auto realcode = code;

        std::ifstream file;
        std::string content;

        try
        {
            file.open(req.uri);
            if (!file.is_open())
                throw ::std::ifstream::failure("Could not open file");
            ssize_t r = 0;
            while ((r = file.readsome(buffer, BUFFER_SIZE)) > 0)
                content += std::string(buffer, r);
            file.close();
            realcode = STATUS_CODE::OK;
        }
        catch (const std::exception &e)
        {
            content = "<p>404 Not found</p>";
            realcode = STATUS_CODE::NOT_FOUND;
            std::cerr << e.what() << '\n';
        }

        if (req.method == Method::HEAD)
            content = "";

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
        response +=
            "Content-Length: " + std::to_string(content.size()) + "\r\n";
        response += "Connection: close\r\n";
        response += "Server: Spider - Pierre da god Edition\r\n";
        response += "\r\n";
        response += content;
    }
} // namespace http
