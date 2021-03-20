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

        std::string carriage = "\r\n\r\n";

        if (response_.find(carriage) != std::string::npos)
        {
#ifdef _DEBUG
            std::cout << response_;
#endif
            std::string str = "Content-Length:";
            auto start = response_.find(str) + str.size();
            auto end = response_.find("\r\n", start);
            content_length_ = std::stoi(std::string(response_.begin() + start,
                                                    response_.begin() + end));
            current_length_ = 0;
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
            response_.erase(rm, response_.find("\r\n", rm));
        }

        // TODO Add Connection header

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