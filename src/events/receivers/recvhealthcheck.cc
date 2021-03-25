#include "recvhealthcheck.hh"

#include "vhost/vhost-reverse-proxy.hh"

namespace http
{
    bool RecvHealthCheckEW::check_response()
    {
        size_t pos = 0;
        if ((pos = data.find("\r\n")) == std::string::npos)
        {
            if (host_)
                host_->alive = true;
            else
                return false;
        }
        return false;
    }

    void RecvHealthCheckEW::operator()()
    {
        try
        {
            char buffer[BUFFER_SIZE];
            size_t received =
                this->backend_sock_.get()->recv(buffer, BUFFER_SIZE);
            if (received <= 0)
            {
                check_response();
                event_register.unregister_ew(this);
                return;
            }
            data += std::string(buffer, 0, received);
            if (check_response())
            {
                event_register.unregister_ew(this);
                return;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }
} // namespace http