#include "config.hh"

#include <fstream>

#include "misc/json.hh"

namespace http
{
    struct ServerConfig parse_configuration(const std::string &path)
    {
        std::ifstream file_in(path);

        json j;

        file_in >> j;

        struct ServerConfig s_conf;

        s_conf.ip = j["ip"];
        s_conf.port = j["port"];
        s_conf.server_name = j["server_name"];
        s_conf.root = j["root"];

        if (j.count("default_file"))
            s_conf.default_file = j["default_file"];

        return s_conf;
    }
} // namespace http
