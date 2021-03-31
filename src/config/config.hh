/**
 * \file config/config.hh
 * \brief Declaration of ServerConfig and VHostConfig.
 */

#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <openssl/ssl.h>
#include <optional>
#include <string>
#include <vector>

namespace http
{
    struct ProxyPassConfig
    {
        ProxyPassConfig() = default;

        std::string ip;
        unsigned int port;
        std::map<std::string, std::string> proxy_set_header;
        std::vector<std::string> proxy_remove_header;
        std::map<std::string, std::string> set_header;
        std::vector<std::string> remove_header;

        std::string upstream = "";
    };

    struct Host
    {
        std::string ip;
        int port;
        std::string health;
        int weight = 1;
        bool alive = false;
    };
    struct Backend
    {
        std::string name;
        std::string method;
        std::vector<std::shared_ptr<Host>> hosts;

        std::vector<size_t> robin_tab;
        int robin_index = -1;

        void create_robin_tab()
        {
            std::vector<size_t> weights;

            for (auto host : hosts)
                weights.push_back(host->weight);

            bool add = true;
            while (add)
            {
                add = false;
                for (size_t i = 0; i < weights.size(); i++)
                {
                    if (weights[i] > 0)
                    {
                        this->robin_tab.push_back(i);
                        weights[i] -= 1;
                        add = true;
                    }
                }
            }

            this->robin_index = 0;
        }

        void robin_index_incr()
        {
            robin_index += 1;
            robin_index %= robin_tab.size();
        }
    };

    /**
     * \struct VHostConfig
     * \brief Value object storing a virtual host configuration.
     *
     * Since each virtual host of the server has its own configuration, a
     * dedicated structure is required to store the information related to
     * each one of them.
     */
    struct VHostConfig
    {
        VHostConfig() = default;
        VHostConfig(const VHostConfig &) = default;
        VHostConfig &operator=(const VHostConfig &) = default;
        VHostConfig(VHostConfig &&) = default;
        VHostConfig &operator=(VHostConfig &&) = default;

        ~VHostConfig() = default;

        // FIXME: Add members to store the information relative to a vhost.
        std::string ip;
        unsigned int port;
        std::string server_name;
        std::string root;
        std::string default_file = "index.html";
        std::string ssl_cert;
        std::string ssl_key;
        std::string auth_basic;
        std::vector<std::string> auth_basic_users;

        std::optional<ProxyPassConfig> proxy_pass = std::nullopt;
        bool default_file_found = false;
        bool default_vhost = false;
        bool auto_index = false;
    };

    /*
     * \brief Value object storing the server configuration.
     *
     * To avoid opening the configuration file each time we need to access the
     * server configuration, a dedicated structure is required to store it.
     */
    struct ServerConfig
    {
        ServerConfig() = default;
        ServerConfig(const ServerConfig &) = default;
        ServerConfig &operator=(const ServerConfig &) = default;
        ServerConfig(ServerConfig &&) = default;
        ServerConfig &operator=(ServerConfig &&) = default;

        ~ServerConfig() = default;

        /* FIXME: Add members to store the information relative to the
           configuration of the server. */
        std::vector<VHostConfig> vhosts;

        std::vector<Backend> upstreams;
    };

    /**
     * \brief Parse the server configuration file.
     *
     * \param path string containing the path to the server configuration
     * file.
     * \return The server configuration.
     */
    struct ServerConfig parse_configuration(const std::string &path);
} // namespace http