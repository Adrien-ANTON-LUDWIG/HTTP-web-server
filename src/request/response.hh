/**
 * \file request/response.hh
 * \brief Response declaration.
 */

#pragma once

#include <fstream>

#include "request/request.hh"
#include "request/types.hh"

namespace http
{
    /**
     * \struct Response
     * \brief Value object representing a response.
     */
    struct Response
    {
        explicit Response(const STATUS_CODE &);
        Response(const struct Request &req,
                 const STATUS_CODE & = STATUS_CODE::OK);

        // Constructor called by RecvResponseEW
        Response(const std::string &resp);

        Response() = default;
        Response(const Response &resp)
        {
            response = resp.response;
            if (resp.file_stream.is_open())
                file_stream = std::ifstream(resp.file_path);
            file_path = resp.file_path;
        }
        Response &operator=(const Response &) = default;
        Response(Response &&) = default;
        Response &operator=(Response &&) = default;
        ~Response() = default;

        Response(const Request &req, const STATUS_CODE &code,
                 std::string &list_directory);

        std::string response;
        std::string file_path = ""; // For copying
        std::ifstream file_stream;
    };
} // namespace http
