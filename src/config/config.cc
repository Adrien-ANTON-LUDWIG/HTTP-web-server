#include "config.hh"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "arpa/inet.h"
#include "misc/json.hh"
#include "misc/openssl/ssl.hh"

namespace http
{
    static void parse_configuration2(struct VHostConfig &vhost,
                                     nlohmann::json &v)
    {
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

        if (v.find("ssl_cert") != v.end() || v.find("ssl_key") != v.end())
        {
            if (v.find("ssl_cert") == v.end() || v.find("ssl_key") == v.end())
            {
                std::cerr << "ssl_cert and ssl_key must both be define.\n";
                exit(1);
            }
            vhost.ssl_cert = v["ssl_cert"];
            vhost.ssl_key = v["ssl_key"];
        }
        if (v.find("auth_basic") != v.end()
            || v.find("auth_basic_users") != v.end())
        {
            if (v.find("auth_basic") == v.end()
                || v.find("auth_basic_users") == v.end())
            {
                std::cerr << "auth_basic and auth_basic_users must "
                             "both be define.\n";
                exit(1);
            }
            vhost.auth_basic = v["auth_basic"];
            // vhost.auth_basic_users = v["auth_basic_users"];
        }
    }

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
            bool default_vhost_find = false;

            for (auto v : vhosts)
            {
                struct VHostConfig vhost;

                parse_configuration2(vhost, v);
                if (v.find("default_vhost") != v.end())
                {
                    if (default_vhost_find)
                    {
                        std::cerr << "default_vhost must be unique in the "
                                     "whole config file.\n";
                        exit(1);
                    }
                    default_vhost_find = true;
                    vhost.default_vhost = true;
                }

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
