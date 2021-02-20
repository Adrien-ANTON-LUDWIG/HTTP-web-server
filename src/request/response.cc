#include "request/response.hh"

#include <istream>
#include <sstream>

namespace http
{
    Response::Response(const STATUS_CODE &)
    {
        /* FIXME */
    }

    Response::Response(const Request &req, const STATUS_CODE &code)
    {
        (void)(req);
        (void)(code);
    }
} // namespace http
