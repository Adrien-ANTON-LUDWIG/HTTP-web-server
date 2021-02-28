#include "dispatcher.hh"

namespace http
{
    void Dispatcher::add_vhost(const shared_vhost &vhost)
    {
        vhosts_.push_back(vhost);
    }

    static void check_host(const shared_connection &connection,
                           struct Request &request, const VHostConfig v_conf)
    {
        std::string host = request.host;
        if (host[0] == '[')
        {
            return;
            host.erase(host.begin());
            host.erase(host.find(']'));
        }
        if (request.host == "" || connection->listener_ip != v_conf.ip
            || connection->listener_port != v_conf.port)
        {
            request.status_code = STATUS_CODE::BAD_REQUEST;
            return;
        }

        if (request.host == connection->listener_ip
            || request.host == v_conf.server_name)
            return;

        auto index = request.host.find_last_of(':');

        if (index == std::string::npos)
        {
            request.status_code = STATUS_CODE::BAD_REQUEST;
            return;
        }

        std::string host_or_ip(request.host.begin(),
                               request.host.begin() + index);
        std::string port(request.host.begin() + index + 1, request.host.end());

        if (((host_or_ip != v_conf.server_name
              && host_or_ip != connection->listener_ip)
             || port != std::to_string(connection->listener_port)))
            request.status_code = STATUS_CODE::BAD_REQUEST;
    }

    void Dispatcher::dispatch(const shared_connection &connection,
                              struct Request &request)
    {
        auto v_conf = vhosts_[0]->conf_get();
        check_host(connection, request, v_conf);

        // FIXME Renvoyer vers le vhost le moins chargé

        vhosts_[0]->respond(request, connection);
    }
} // namespace http