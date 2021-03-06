/**
 * \file request/request.hh
 * \brief Request declaration.
 */

#pragma once

#include <cctype>
#include <iostream>
#include <map>
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

        void build_uri();

        void parse_method(const std::string &method_string);

        void check_content_length(std::string &value);

        void register_header(const std::string &name, std::string value);

        void parse_headers(std::stringstream ss);

        void parse_protocol_version(std::string http_version);

        void parse_request(const std::string &message);

        bool is_good();

        void pretty_print();

        Method method;
        std::string uri;
        std::map<std::string, std::string> headers;

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
