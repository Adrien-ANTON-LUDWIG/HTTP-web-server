/**
 * \file config/config.hh
 * \brief Declaration of ServerConfig and VHostConfig.
 */

#pragma once

#include <map>
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
    };

    struct BackendConfig
    {
        std::string ip;
        int port;
        std::string health;
        int weight;
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

        bool default_vhost = false;
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

        std::vector<BackendConfig> backends;
        std::string balancing_method;
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