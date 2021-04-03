#include "recvresponse.hh"

#include <regex>

#include "vhost/vhost-reverse-proxy.hh"
namespace http
{
    void RecvResponseEW::unregister_proxy_timeout()
    {
        if (connection_->timeout_proxy != nullptr)
        {
            event_register.get_loop().unregister_timer_watcher(
                connection_->timeout_proxy->get_et().get());
        }
    }

    void RecvResponseEW::recv_headers()
    {
        char buffer[BUFFER_SIZE];
        auto read_size = backend_sock_->recv(buffer, BUFFER_SIZE);

        if (read_size == -1)
        {
            unregister_proxy_timeout();
            event_register.unregister_ew(this);
            return;
        }

        response_.append(buffer, buffer + read_size);

        auto carriage = response_.find("\r\n\r\n");

        if (carriage != std::string::npos)
        {
#ifdef _DEBUG
            std::cout << "RecvResponseEW response:\n" << response_ << "\n";
#endif
            std::string str = "Content-Length:";
            if (response_.find(str) != std::string::npos)
            {
                auto start = response_.find(str) + str.size();
                auto end = response_.find("\r\n", start);
                content_length_ = std::stoi(std::string(
                    response_.begin() + start, response_.begin() + end));
            }
            // TODO Body
            body_ = response_.substr(carriage + 4);
            current_length_ = body_.size();
            response_.erase(carriage + 2);
        }
    }

    void RecvResponseEW::recv_body()
    {
        char buffer[BUFFER_SIZE];
        auto read_size = backend_sock_->recv(buffer, BUFFER_SIZE);

        if (read_size <= 0)
        {
            recv_finished = true;
            unregister_proxy_timeout();
            event_register.unregister_ew(this);
            std::cout << "RecvResponse : recv_body end" << std::endl;
            return;
        }

        body_ += std::string(buffer, read_size);
        current_length_ += read_size;
    }

    std::string RecvResponseEW::build_response()
    {
        std::string res;

        for (auto &header : connection_->vhost_conf.proxy_pass->remove_header)
        {
            auto rm = response_.find(header);
            if (rm != std::string::npos)
                response_.erase(rm, response_.find("\r\n", rm) + 2);
        }

        if (connection_->keep_alive)
        {
            auto rm = response_.find("Connection");
            if (rm != std::string::npos)
                response_.erase(rm, response_.find("\r\n", rm) + 2);
            response_ += "Connection: keep-alive\r\n";
        }

        for (auto &header : connection_->vhost_conf.proxy_pass->set_header)
        {
            auto rm = response_.find(header.first);
            if (rm != std::string::npos)
                response_.erase(rm, response_.find("\r\n", rm) + 2);
            response_ += header.first + ": " + header.second + "\r\n";
        }

        if (response_.find("\r\nDate:") == std::string::npos)
        {
            time_t now;
            char datebuffer[BUFFER_SIZE];
            time(&now);
            auto now_time = gmtime(&now);

            size_t time_size = strftime(datebuffer, BUFFER_SIZE,
                                        "%a, %d %b %Y %X %Z\r\n", now_time);
            response_ += "Date: " + std::string(datebuffer, time_size) + "\r\n";
        }

        if (response_.find("\r\nContent-Length:") == std::string::npos)
        {
            response_ +=
                "Content-Length: " + std::to_string(content_length_) + "\r\n";
        }

        response_ += "\r\n";
        res = response_ + body_;

        return res;
    }

    static bool check_response(std::string response)
    {
        std::regex e("HTTP/1.1 [0-9]{3} [a-zA-Z ]*");
        return std::regex_search(response, e);
    }

    void RecvResponseEW::operator()()
    {
        if (content_length_ == -1)
            recv_headers();
        else if (content_length_ != -1)
            recv_body();

        if (recv_finished)
        {
            auto response = build_response();
            if (!check_response(response))
                response = Response(STATUS_CODE::BAD_GATEWAY).response;
            unregister_proxy_timeout();
            event_register.register_event<SendResponseEW>(connection_,
                                                          Response(response));
            event_register.unregister_ew(this);
        }
    }
} // namespace http