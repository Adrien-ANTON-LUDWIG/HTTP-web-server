#include "config.hh"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>

#include "arpa/inet.h"
#include "misc/json.hh"
#include "misc/openssl/ssl.hh"
#include "vhost/dispatcher.hh"

namespace http
{
    static void parse_essential(struct VHostConfig &vhost, nlohmann::json &v)
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
        if (v.find("proxy_pass") != v.end() && v.find("root") != v.end())
        {
            std::cerr << "Root and proxy_pass are mutually exclusive\n";
            exit(0);
        }
        if (v.find("proxy_pass") == v.end())
        {
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
        }
    }

    static void parse_ssl_auth(struct VHostConfig &vhost, nlohmann::json &v)
    {
        if (v.find("ssl_cert") != v.end() || v.find("ssl_key") != v.end())
        {
            if (v.find("ssl_cert") == v.end() || v.find("ssl_key") == v.end())
            {
                std::cerr << "ssl_cert and ssl_key must both be define.\n";
                exit(1);
            }

            vhost.ssl_cert = v["ssl_cert"];
            vhost.ssl_key = v["ssl_key"];

            if (vhost.ssl_cert.empty() || vhost.ssl_key.empty())
            {
                std::cerr << "ssl_cert or ssl_key is empty.\n";
                exit(1);
            }

            if (!std::filesystem::exists(vhost.ssl_cert)
                || !std::filesystem::exists(vhost.ssl_key))
            {
                std::cerr << "ssl_cert or ssl_key does not exist.\n";
                exit(1);
            }
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
            auto l = v["auth_basic_users"];
            for (std::string e : l)
            {
#ifdef _DEBUG
                std::cout << e << '\n';
#endif
                vhost.auth_basic_users.push_back(e);
                size_t pos = e.find(":");
                if (pos == std::string::npos || pos == 0 || e[pos + 1] == '\0')
                {
                    std::cerr
                        << "auth_basic_users wrong format : user:password\n";
                    exit(1);
                }
                for (auto c : e)
                {
                    if (c > 32 && c < 127)
                        continue;
                    else
                    {
                        std::cerr << "invalid character";
                        exit(1);
                    }
                }
            }
            if (vhost.auth_basic.empty() || vhost.auth_basic_users.size() == 0)
            {
                std::cerr << "auth_basic or auth_basic_users is empty.\n";
                exit(1);
            }
        }
    }

    static void parse_reverse_proxy(struct VHostConfig &vhost,
                                    nlohmann::json &v)
    {
        struct ProxyPassConfig proxy;
        if (v.find("proxy_pass") != v.end())
        {
            v = *v.find("proxy_pass");

            if (v.find("upstream") != v.end())
            {
                proxy.upstream = v["upstream"];
                vhost.proxy_pass = proxy;
                return;
            }

            proxy.ip = v["ip"];
            int port = v["port"];

            if (port < 0 || port > 65535)
            {
                std::cerr << "Forbidden port\n";
                exit(1);
            }
            proxy.port = port;

            std::vector<std::string> proxy_remove_header =
                v["proxy_remove_header"];
            proxy.proxy_remove_header = proxy_remove_header;
            std::map<std::string, std::string> proxy_set_header =
                v["proxy_set_header"];
            proxy.proxy_set_header = proxy_set_header;
            std::vector<std::string> remove_header = v["remove_header"];
            proxy.remove_header = remove_header;
            std::map<std::string, std::string> set_header = v["set_header"];
            proxy.set_header = set_header;

            vhost.proxy_pass = proxy;
        }
    }
    static void parse_upstream(struct ServerConfig &config, nlohmann::json &j)
    {
        (void)config;
        if (j.find("upstreams") == j.end())
            return;
        auto upstreams = *j.find("upstreams");
        for (auto it = upstreams.begin(); it != upstreams.end(); it++)
        {
            struct Backend backend;
            backend.name = it.key();
            if (it.value().find("method") == it.value().end())
            {
                std::cerr << "No method declared\n";
                exit(1);
            }
            backend.method = *it.value().find("method");
            if (it.value().find("hosts") == it.value().end())
            {
                std::cerr << "No hosts declared\n";
                exit(1);
            }
            int i = 0;
            for (auto &h : *it.value().find("hosts"))
            {
                i++;
                struct Host host;

                host.ip = h["ip"];
                host.port = h["port"];
                if (h.find("weight") != h.end())
                    host.weight = h["weight"];
                if (backend.method == "fail-robin"
                    || backend.method == "failover")
                    host.health = h["health"];
                else
                {
                    if (h.find("health") != h.end())
                        host.health = h["health"];
                }

                backend.hosts.push_back(std::make_shared<Host>(host));
            }
            if (i == 0)
            {
                std::cerr << "Hosts is empty\n";
                exit(1);
            }
            backend.create_robin_tab();
            config.upstreams.push_back(backend);
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

            for (auto &v : vhosts)
            {
                struct VHostConfig vhost;

                parse_essential(vhost, v);
                parse_ssl_auth(vhost, v);
                if (v.find("default_vhost") != v.end())
                {
                    bool value = v["default_vhost"];
                    if (default_vhost_find && value)
                    {
                        std::cerr << "default_vhost must be unique in the "
                                     "whole config file.\n";
                        exit(1);
                    }
                    default_vhost_find = value;
                    vhost.default_vhost = value;
                }

                parse_reverse_proxy(vhost, v);

                for (auto c : s_conf.vhosts)
                {
                    if (c.ip == vhost.ip && c.port == vhost.port
                        && c.server_name == vhost.server_name
                        && (c.ssl_cert == vhost.ssl_cert
                            || c.ssl_key == vhost.ssl_key))
                    {
                        std::cerr << "There is already a vhost with the same "
                                     "configuration\n";
                        exit(1);
                    }
                }
                s_conf.vhosts.push_back(vhost);
            }
            parse_upstream(s_conf, j);
            return s_conf;
        }
        catch (json::exception &e)
        {
            std::cerr << e.what() << "\n";
            exit(1);
        }
    }
} // namespace http
