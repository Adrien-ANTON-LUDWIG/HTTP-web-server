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

#define NB_OF_METHODS 9

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
        PUT,
        DELETE,
        CONNECT,
        OPTIONS,
        TRACE,
        PATCH,
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
            std::string methods[NB_OF_METHODS] = {
                "GET",     "HEAD",    "POST",  "PUT",  "DELETE",
                "CONNECT", "OPTIONS", "TRACE", "PATCH"
            };

            int i = 0;
            for (; i < NB_OF_METHODS; i++)
                if (method_string == methods[i])
                {
                    method = static_cast<Method>(i);
                    break;
                }

            if (i == NB_OF_METHODS)
            {
                method = Method::ERR;
                status_code = STATUS_CODE::BAD_REQUEST;
            }
            else if (method > Method::POST)
                status_code = STATUS_CODE::METHOD_NOT_ALLOWED;
        }

        void parse_headers(const std::string &message)
        {
            std::stringstream ss(message);
            std::string method_string;
            std::string http_version;
            ss >> method_string;
            ss >> uri;

            if (uri[0] != '/')
                status_code = STATUS_CODE::BAD_REQUEST;

            ss >> http_version;

            parse_method(method_string);
            if (http_version != "HTTP/1.1")
                status_code = STATUS_CODE::UPGRADE_REQUIRED;
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
                    if (name == "Host")
                    {
                        if (host == "")
                            host = value;
                        else
                            status_code = STATUS_CODE::BAD_REQUEST;
                    }
                    else
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
            std::string methods[NB_OF_METHODS + 1] = {
                "GET",     "HEAD",    "POST",  "PUT",   "DELETE",
                "CONNECT", "OPTIONS", "TRACE", "PATCH", "ERR"
            };

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

        std::string host;

        size_t content_length = 0;
        std::string body;

        /**
         * \brief Status code of the request
         */
        STATUS_CODE status_code = STATUS_CODE::OK;
    };
} // namespace http
