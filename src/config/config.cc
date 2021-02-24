#include "config.hh"

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

                /*
                unsigned char buf[sizeof(struct in6_addr)];
                int version = vhost.ip.find('.') != std::string::npos;

                int ip_status = inet_pton(version, vhost.ip.c_str(), buf);

                if (ip_status <= 0)
                {
                    std::cerr << "Bad ip\n";
                    exit(1);
                }
                */

                int port = v["port"];

                if (port < 0 || port > 65535)
                {
                    std::cerr << "Forbidden port\n";
                    exit(1);
                }

                vhost.port = port;
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
