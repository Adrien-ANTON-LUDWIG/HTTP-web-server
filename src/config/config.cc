#include "config.hh"

#include <fstream>
#include <iostream>

#include "misc/json.hh"

namespace http
{
    struct ServerConfig parse_configuration(const std::string &path)
    {
        try
        {
            std::ifstream file_in(path);

            struct ServerConfig s_conf;
            json j;
            file_in >> j;

            if (j.find("vhosts") == j.end())
                exit(1);

            auto vhosts = *j.find("vhosts");

            for (auto v : vhosts)
            {
                struct VHostConfig vhost;
                vhost.ip = v["ip"];
                vhost.port = v["port"];
                vhost.server_name = v["server_name"];
                vhost.root = v["root"];

                if (v.find("default_file") != v.end())
                    vhost.default_file = v["default_file"];

                s_conf.vhosts.push_back(vhost);
            }

            return s_conf;
        }
        catch (json::exception &e)
        {
            std::cerr << e.what() << "\n";
            exit(1);
        }
    }
} // namespace http
