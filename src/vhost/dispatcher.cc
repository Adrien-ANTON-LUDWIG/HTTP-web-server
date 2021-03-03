#include "dispatcher.hh"

namespace http
{
    void Dispatcher::add_vhost(const shared_vhost &vhost)
    {
        vhosts_.push_back(vhost);
    }

    static bool check_host(const shared_connection &connection,
                           struct Request &request, const VHostConfig v_conf)
    {
        std::string host = request.host;
        if (host == "" || connection->listener_ip != v_conf.ip
            || connection->listener_port != v_conf.port)
        {
            return false;
        }

        if (host[0] == '[')
        {
            host.erase(host.begin());
            host.erase(host.find(']'));
        }

        if (host == connection->listener_ip || host == v_conf.server_name)
            return true;

        auto index = host.find_last_of(':');

        if (index == std::string::npos)
            return false;

        std::string host_or_ip(host.begin(), host.begin() + index);
        std::string port(host.begin() + index + 1, host.end());

        if (((host_or_ip != v_conf.server_name
              && host_or_ip != connection->listener_ip)
             || port != std::to_string(connection->listener_port)))
            return false;
        return true;
    }

    void Dispatcher::dispatch(const shared_connection &connection,
                              struct Request &request)
    {
        shared_vhost vhost = nullptr;
        for (auto v : vhosts_)
        {
            if (check_host(connection, request, v->conf_get()))
            {
                vhost = v;
                break;
            }
        }
        if (vhost == nullptr)
        {
            request.status_code = STATUS_CODE::BAD_REQUEST;
            vhosts_[0]->respond(request, connection);
        }
        else
            vhost->respond(request, connection);
    }
} // namespace http