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

        Response() = default;
        Response(const Response &resp)
        {
            response = resp.response;
            file_stream = std::ifstream(resp.file_path);
            file_path = resp.file_path;
        }
        Response &operator=(const Response &) = default;
        Response(Response &&) = default;
        Response &operator=(Response &&) = default;
        ~Response() = default;

        std::string response;
        std::string file_path; // For copying
        std::ifstream file_stream;
    };
} // namespace http
