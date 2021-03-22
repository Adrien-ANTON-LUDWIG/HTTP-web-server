#include "recvresponse.hh"

namespace http
{
    void RecvResponseEW::recv_headers()
    {
        char buffer[BUFFER_SIZE];
        auto read_size = backend_sock_->recv(buffer, BUFFER_SIZE);

        if (read_size == -1)
        {
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
            auto start = response_.find(str) + str.size();
            auto end = response_.find("\r\n", start);
            content_length_ = std::stoi(std::string(response_.begin() + start,
                                                    response_.begin() + end));

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
            event_register.unregister_ew(this);
            std::cout << "RecvResponse : recv_body error" << std::endl;
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

        response_ += "\r\n";
        res = response_ + body_;

        return res;
    }

    void RecvResponseEW::operator()()
    {
        if (content_length_ == -1)
            recv_headers();
        else
            recv_body();

        if (content_length_ <= current_length_)
        {
            auto response = build_response();
            event_register.register_event<SendResponseEW>(connection_,
                                                          Response(response));
            event_register.unregister_ew(this);
        }
    }
} // namespace http