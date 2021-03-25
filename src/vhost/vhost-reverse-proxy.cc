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
                sfd->connect(rp.ai_addr, rp.ai_addrlen);
                sfd->ipv6_set(rp.ai_family == AF_INET6);
                break;
            }
            catch (const std::exception &)
            {
                sfd = nullptr;
                continue;
            }
        }
        if (!sfd)
            return nullptr;
        if (sfd->fd_get().get()->fd_ == -1)
        {
            std::cerr << "Could not connect to any interface\n";
            exit(0);
        }
        sfd->set_hostname(ip.c_str());
        return sfd;
    }

    void
    VHostReverseProxy::build_request(Request &request,
                                     std::shared_ptr<Connection> &connection)
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
            {
                forwarded->second += x_for_to_forwarded(x_for->second);
                request.headers.erase("X-Forwarded-For");
            }
            else if (!has_x_for && has_x_host && !has_x_proto)
            {
                x_host->second += ",";
                forwarded->second += std::regex_replace(
                    x_host->second, std::regex("([^ ,]*,)"), "host=$1");
                request.headers.erase("X-Forwarded-Host");
            }
            else if (!has_x_for && !has_x_host && has_x_proto)
            {
                x_proto->second += ",";
                forwarded->second += std::regex_replace(
                    x_proto->second, std::regex("([^ ,]*,)"), "proto=$1");
                request.headers.erase("X-Forwarded-Proto");
            }
        }

        forwarded->second += "for=";
        if (connection->sock->is_ipv6())
            forwarded->second +=
                "\"[" + connection->sock->get_hostname() + "]\"";
        else
            forwarded->second += connection->sock->get_hostname();

        forwarded->second += ";host=" + request.headers["Host"]
            + ";proto=" + (conf_.ssl_cert.empty() ? "http" : "https");

        request.headers["Host"] = request.host;
    }

    void VHostReverseProxy::handle_round_robin()
    {
        conf_.proxy_pass->ip =
            backend->hosts[backend->robin_tab[backend->robin_index]]->ip;
        conf_.proxy_pass->port =
            backend->hosts[backend->robin_tab[backend->robin_index]]->port;

        std::cout << conf_.proxy_pass->ip << ": " << conf_.proxy_pass->port
                  << '\n';

        backend->robin_index_incr();
    }

    void
    VHostReverseProxy::handle_failover(Request &request,
                                       std::shared_ptr<Connection> connection)
    {
        size_t i = 0;
        while (i < backend->hosts.size() && !backend->hosts[i]->alive)
        {
            i++;
        }

        if (i == backend->hosts.size())
        {
            request.status_code = STATUS_CODE::SERVICE_UNAVAILABLE;
            event_register.register_event<SendResponseEW>(
                connection,
                Response(request, STATUS_CODE::SERVICE_UNAVAILABLE));
            return;
        }

        conf_.proxy_pass->ip = backend->hosts[i]->ip;
        conf_.proxy_pass->port = backend->hosts[i]->port;
    }

    void
    VHostReverseProxy::handle_fail_robin(Request &request,
                                         std::shared_ptr<Connection> connection)
    {
        auto i = backend->robin_index;
        backend->robin_index_incr();

        while (backend->robin_index != i
               && !backend->hosts[backend->robin_index]->alive)
            backend->robin_index_incr();

        if (i == backend->robin_index)
        {
            request.status_code = STATUS_CODE::SERVICE_UNAVAILABLE;
            event_register.register_event<SendResponseEW>(
                connection,
                Response(request, STATUS_CODE::SERVICE_UNAVAILABLE));
            return;
        }

        conf_.proxy_pass->ip =
            backend->hosts[backend->robin_tab[backend->robin_index]]->ip;
        conf_.proxy_pass->port =
            backend->hosts[backend->robin_tab[backend->robin_index]]->port;

        std::cout << conf_.proxy_pass->ip << ": " << conf_.proxy_pass->port
                  << '\n';

        backend->robin_index_incr();
    }

    void VHostReverseProxy::respond(Request &request,
                                    std::shared_ptr<Connection> connection)
    {
        // Authentification
        if (request.status_code == STATUS_CODE::PROXY_AUTHENTICATION_REQUIRED)
        {
            event_register.register_event<SendResponseEW>(
                connection, Response(request, request.status_code));
            return;
        }

        // LOAD BALANCING
        if (backend)
        {
            if (backend->method == "round-robin")
                handle_round_robin();
            else if (backend->method == "failover")
                handle_failover(request, connection);
            else if (backend->method == "fail-robin")
                handle_fail_robin(request, connection);
        }

        build_request(request, connection);

        connection->vhost_conf = conf_;
        event_register.register_event<SendRequestEW>(
            request,
            connect_to_backend(conf_.proxy_pass->ip, conf_.proxy_pass->port),
            connection);
    }

    void VHostReverseProxy::timeout_cb(struct ev_loop *loop, ev_timer *et,
                                       int revents)
    {
        (void)loop;
        (void)revents;
        auto be = *static_cast<std::shared_ptr<Backend> *>(et->data);

        for (auto &host : be->hosts)
        {
            unsigned int port = host->port;
            host->alive = false;
            shared_socket backend_sock = connect_to_backend(host->ip, port);
            if (!backend_sock)
                continue;
            shared_connection connection =
                std::make_shared<Connection>(backend_sock, host->ip, port);

            event_register.register_event<SendHealthCheckEW>(host, backend_sock,
                                                             connection);
        }
    }
} // namespace http
