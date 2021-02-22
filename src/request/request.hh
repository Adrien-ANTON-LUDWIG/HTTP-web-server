/**
 * \file request/request.hh
 * \brief Request declaration.
 */

#pragma once

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "request/response.hh"
#include "request/types.hh"
namespace http
{
    /**
     * @brief Http methods enum
     *
     */
    enum class Method
    {
        GET,
        HEAD,
        POST,
        ERR
    };

    /**
     * \struct Request
     * \brief Value ob=ject representing a request.
     */
    struct Request
    {
        Request() = default;
        Request(const Request &) = default;
        Request &operator=(const Request &) = default;
        Request(Request &&) = default;
        Request &operator=(Request &&) = default;
        ~Request() = default;

        void parse_method(const std::string &method_string)
        {
            if (method_string == "GET")
                method = Method::GET;
            else if (method_string == "HEAD")
                method = Method::HEAD;
            else if (method_string == "POST")
                method = Method::POST;
            else
                method = Method::ERR;
        }

        void parse_headers(const std::string &message)
        {
            std::stringstream ss(message);
            std::string method_string;
            std::string http_version;
            ss >> method_string;
            ss.peek();
            if (ss.peek() != ' ')
                std::cerr << "Bad request\n";
            ss >> uri;
            ss.peek();
            if (ss.peek() != ' ')
                std::cerr << "Bad request\n";
            ss >> http_version;

            parse_method(method_string);
            if (http_version != "HTTP/1.1")
                std::cerr << "Bad version";

            std::string line;
            while (getline(ss, line) && line != "")
            {
                std::string name;
                std::string value;
                if (line.find(":") != std::string::npos)
                {
                    auto pos = line.find(":");
                    name = line.substr(0, pos++);

                    while (isspace(line[pos]))
                        pos++;
                    value = line.substr(pos, line.size() - pos);
                    if (name == "Content-Length")
                        content_length = stoi(value);
                    headers.push_back(
                        std::pair<std::string, std::string>(name, value));
                }
            }
            body = message.substr(message.find("\r\n\r\n") + 4);

            if (body != "" && content_length == 0)
                std::cerr << "\nUnexpected body\n";
        }

        void pretty_print()
        {
            std::string methods[4] = { "GET", "HEAD", "POST", "ERROR" };
            std::cout << methods[static_cast<int>(method)] << ' ' << uri << ' '
                      << "HTTP 1.1\n";

            for (auto h : headers)
                std::cout << h.first << ": " << h.second << '\n';

            if (body != "")
                std::cout << '\n' << body << '\n';
        }

        Method method;
        std::string uri;
        std::vector<std::pair<std::string, std::string>> headers;

        size_t content_length = 0;
        std::string body;
    };
} // namespace http
