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

        void build_uri()
        {
            auto prefix_pos = uri.find(':');
            if (prefix_pos != std::string::npos)
            {
                std::string prefix = uri.substr(0, prefix_pos);
                if (prefix == "http")
                {
                    uri.erase(0, prefix_pos + 3);
                    auto authority = uri.find('/');
                    host = uri.substr(0, authority);
                    uri.erase(0, authority);
                }
            }

            auto query = uri.find('?');
            if (query != std::string::npos)
            {
                uri.erase(query, uri.length());
            }

            auto fragment = uri.find('#');
            if (fragment != std::string::npos)
            {
                uri.erase(fragment, uri.length());
            }
        }

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

        void parse_headers(std::stringstream ss)
        {
            std::string line;
            bool cl = false;
            while (getline(ss, line) && line != "")
            {
                std::string name;
                std::string value;
                if (line[line.size() - 1] == '\r')
                    line.erase(line.size() - 1);
                if (line.find(":") != std::string::npos)
                {
                    auto pos = line.find(":");
                    name = line.substr(0, pos++);

                    while (isspace(line[pos]))
                        pos++;
                    value = line.substr(pos, line.size() - pos);
                    if (name == "Content-Length")
                    {
                        if (cl)
                        {
                            status_code = STATUS_CODE::BAD_REQUEST;
                            content_length = 0;
                        }
                        else
                        {
                            cl = true;
                            long long len = stoll(value);
                            if (len < 0)
                            {
                                status_code = STATUS_CODE::BAD_REQUEST;
                                content_length = 0;
                            }
                            else
                                content_length = len;
                        }
                    }
                    if (name == "Host")
                    {
                        if (host == "")
                        {
                            while (value[value.size() - 1] == ' ')
                                value.pop_back();
                            host = value;
                        }
                        else
                            status_code = STATUS_CODE::BAD_REQUEST;
                    }
                    else
                        headers.push_back(
                            std::pair<std::string, std::string>(name, value));
                }
            }
        }

        void parse_request(const std::string &message)
        {
            std::stringstream ss(message);
            std::string method_string;
            std::string http_version;
            ss >> method_string;
            ss >> uri;
            ss >> http_version;

            parse_method(method_string);
            try
            {
                if (http_version.find("/") == std::string::npos
                    || http_version.substr(0, http_version.find("/")) != "HTTP")
                    status_code = STATUS_CODE::BAD_REQUEST;
                std::string version = http_version
                                          .substr(http_version.find("/") + 1,
                                                  http_version.size() - 1)
                                          .c_str();
                if (version.size() != 3)
                    status_code = STATUS_CODE::BAD_REQUEST;
                else if (std::stof(version) < 1.1)
                    status_code = STATUS_CODE::UPGRADE_REQUIRED;
            }
            catch (const std::exception &e)
            {
                status_code = STATUS_CODE::BAD_REQUEST;
            }

            parse_headers(std::move(ss));
            body = message.substr(message.find("\r\n\r\n") + 4);
            if (body != "" && content_length == 0)
                std::cerr << "\nUnexpected body\n";

            build_uri();
            if (uri[0] != '/')
                status_code = STATUS_CODE::BAD_REQUEST;
        }

        void pretty_print()
        {
#ifdef _DEBUG
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
#endif
        }

        Method method;
        std::string uri;
        std::vector<std::pair<std::string, std::string>> headers;

        std::string host;

        size_t content_length = 0;
        size_t current_length = 0;
        std::string body;

        /**
         * \brief Status code of the request
         */
        STATUS_CODE status_code = STATUS_CODE::OK;
    };
} // namespace http
