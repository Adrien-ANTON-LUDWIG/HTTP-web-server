#include "vhost-reverse-proxy.hh"

#include <regex>

namespace http
{
    static std::string x_for_to_forwarded(const std::string &x_for)
    {
        auto new_for = x_for + ",";
        new_for =
            std::regex_replace(new_for, std::regex("([^ ,]*),"), "for=$1,");
        new_for = std::regex_replace(new_for, std::regex("=([0-9a-f:]*),"),
                                     "=\"[$1]\",");
        return new_for;
    }

    static shared_socket connect_to_backend(std::string &ip, unsigned int &port)
    {
        auto hint = misc::AddrInfoHint();
        hint.family(AF_UNSPEC);
        hint.socktype(SOCK_STREAM);

        misc::AddrInfo addrinfo =
            misc::getaddrinfo(ip.c_str(), std::to_string(port).c_str(), hint);

        shared_socket sfd = nullptr;
        for (auto rp : addrinfo)
        {
            try
            {
                sfd = std::make_shared<DefaultSocket>(
                    rp.ai_family, rp.ai_socktype, rp.ai_protocol);
                sfd->setsockopt(SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 1);
                sfd->connect(rp.ai_addr, rp.ai_addrlen);
                break;
            }
            catch (const std::exception &)
            {
                continue;
            }
        }
        if (sfd->fd_get().get()->fd_ == -1)
        {
            std::cerr << "Could not connect to any interface\n";
            exit(0);
        }
        sfd->set_hostname(ip.c_str());
        return sfd;
    }

    void VHostReverseProxy::respond(Request &request,
                                    std::shared_ptr<Connection> connection)
    {
        bool find_host_header = false;
        for (auto header : conf_.proxy_pass->proxy_remove_header)
            request.headers.erase(header);

        if (request.headers.find("Connection") != request.headers.end())
            request.headers.erase("Connection");

        for (auto header : conf_.proxy_pass->proxy_set_header)
        {
            if (header.first == "Host")
            {
                find_host_header = true;
                request.host = header.second;
            }
            request.headers[header.first] = header.second;
        }
        if (!find_host_header)
            request.host = conf_.proxy_pass->ip;

        auto forwarded = request.headers.find("Forwarded");
        if (forwarded != request.headers.end())
        {
            forwarded->second += ",";
        }
        else
        {
            request.headers["Forwarded"] = "";
            forwarded = request.headers.find("Forwarded");
            auto x_for = request.headers.find("X-Forwarded-For");
            auto has_x_for = x_for != request.headers.end();
            auto x_host = request.headers.find("X-Forwarded-Host");
            auto has_x_host = x_host != request.headers.end();
            auto x_proto = request.headers.find("X-Forwarded-Proto");
            auto has_x_proto = x_proto != request.headers.end();

            if (has_x_for && !has_x_host && !has_x_proto)
                forwarded->second += x_for_to_forwarded(x_for->second);
            else if (!has_x_for && has_x_host && !has_x_proto)
                forwarded->second +=
                    std::regex_replace(x_for->second,
                                       std::regex("(([^ ,]*,)|([^ ,]*))"),
                                       "host=$1")
                    + ",";
            else if (!has_x_for && !has_x_host && has_x_proto)
                forwarded->second +=
                    std::regex_replace(x_proto->second,
                                       std::regex("(([^ ,]*,)|([^ ,]*))"),
                                       "proto=$1")
                    + ",";
        }

        forwarded->second += "for=";
        if (connection->sock->is_ipv6())
            forwarded->second +=
                "\"[" + connection->sock->get_hostname() + "]\"";
        else
            forwarded->second += connection->sock->get_hostname();
        std::cout << "request.host:" << request.headers["Host"] << '\n';

        forwarded->second += ";host=" + request.headers["Host"]
            + ";proto=" + (conf_.ssl_cert.empty() ? "http" : "https");

        request.headers["Host"] = request.host;

        connection->vhost_conf = conf_;
        event_register.register_event<SendRequestEW>(
            request,
            connect_to_backend(conf_.proxy_pass->ip, conf_.proxy_pass->port),
            connection);
    }

} // namespace http
