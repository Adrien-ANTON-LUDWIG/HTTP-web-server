#include "dispatcher.hh"

#include "misc/openssl/base64.hh"

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
        if (connection->listener_ip != v_conf.ip
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
        shared_vhost vhost = get_default_vhost();
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
        {
            if (!vhost->conf_get().auth_basic.empty())
            {
                if (request.is_proxy
                    != vhost->conf_get().proxy_pass.has_value())
                {
                    request.status_code =
                        vhost->conf_get().proxy_pass.has_value()
                        ? STATUS_CODE::PROXY_AUTHENTICATION_REQUIRED
                        : STATUS_CODE::UNAUTHORIZED;
                }
                else
                {
                    request.auth_basic = vhost->conf_get().auth_basic;
                    if (request.auth.empty())
                    {
                        request.status_code =
                            vhost->conf_get().proxy_pass.has_value()
                            ? STATUS_CODE::PROXY_AUTHENTICATION_REQUIRED
                            : STATUS_CODE::UNAUTHORIZED;
                    }
                    std::stringstream auth_stream(request.auth);
                    std::string auth_type;
                    auth_stream >> auth_type;
                    if (auth_type != "Basic")
                        request.status_code =
                            vhost->conf_get().proxy_pass.has_value()
                            ? STATUS_CODE::PROXY_AUTHENTICATION_REQUIRED
                            : STATUS_CODE::UNAUTHORIZED;
                    else
                    {
                        try
                        {
                            std::string auth_credentials;
                            auth_stream >> auth_credentials;
                            auto plain_credentials =
                                ssl::base64_decode(auth_credentials);
                            bool logged_in = false;
                            for (auto x : vhost->conf_get().auth_basic_users)
                            {
                                if (x == plain_credentials)
                                {
                                    logged_in = true;
                                    break;
                                }
                            }
                            if (!logged_in)
                                request.status_code =
                                    vhost->conf_get().proxy_pass.has_value()
                                    ? STATUS_CODE::PROXY_AUTHENTICATION_REQUIRED
                                    : STATUS_CODE::UNAUTHORIZED;
                        }
                        catch (std::exception &e)
                        {
                            std::cerr
                                << "Decode auhtentification : " << e.what()
                                << '\n';
                            request.status_code =
                                vhost->conf_get().proxy_pass.has_value()
                                ? STATUS_CODE::PROXY_AUTHENTICATION_REQUIRED
                                : STATUS_CODE::UNAUTHORIZED;
                        }
                    }
                }
            }
            vhost->respond(request, connection);
        }
    }

    void Dispatcher::set_default_vhost(const shared_vhost &default_vhost)
    {
        default_vhost_ = default_vhost;
    }

    shared_vhost Dispatcher::get_default_vhost()
    {
        return default_vhost_;
    }

} // namespace http