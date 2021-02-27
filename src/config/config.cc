#include "config.hh"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "arpa/inet.h"
#include "misc/json.hh"

namespace http
{
    struct ServerConfig parse_configuration(const std::string &path)
    {
        try
        {
            std::ifstream file_in(path);
            if (!file_in.is_open())
            {
                std::cerr << "Unknown file\n";
                exit(1);
            }
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

                int port = v["port"];

                if (port < 0 || port > 65535)
                {
                    std::cerr << "Forbidden port\n";
                    exit(1);
                }

                vhost.port = port;
                vhost.server_name = v["server_name"];
                vhost.root = v["root"];

                if (!std::filesystem::is_directory(vhost.root))
                {
                    std::cerr << "Root is not a directory\n";
                    exit(1);
                }
                else if (vhost.root[vhost.root.size() - 1] == '/')
                    vhost.root.pop_back();

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
