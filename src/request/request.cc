#include "request.hh"

namespace http
{
    void Request::build_uri()
    {
        auto prefix_pos = uri.find(':');
        if (prefix_pos != std::string::npos)
        {
            std::string prefix = uri.substr(0, prefix_pos);
            if (prefix == "http")
            {
                uri.erase(0, prefix_pos + 3);
                auto authority = uri.find('/');
                host = uri.substr(0, authority);
                uri.erase(0, authority);
            }
        }

        size_t pos = 0;
        while ((pos = uri.find("/..")) != std::string::npos)
        {
            uri.erase(pos, 3);
            if (uri.size() == 0 || uri[0] != '/')
                uri = "/" + uri;
        }

        auto query = uri.find('?');
        if (query != std::string::npos)
        {
            uri.erase(query, uri.length());
        }

        auto fragment = uri.find('#');
        if (fragment != std::string::npos)
        {
            uri.erase(fragment, uri.length());
        }
    }

    void Request::parse_method(const std::string &method_string)
    {
        /*std::string methods[NB_OF_METHODS] = { "GET",     "HEAD",   "POST",
                                               "PUT",     "DELETE", "CONNECT",
                                               "OPTIONS", "TRACE",  "PATCH" };*/

        std::string methods[NB_OF_METHODS] = { "GET", "HEAD", "POST" };

        int i = 0;
        for (; i < NB_OF_METHODS; i++)
            if (method_string == methods[i])
            {
                method = static_cast<Method>(i);
                break;
            }

        if (i == NB_OF_METHODS)
        {
            method = Method::ERR;
            status_code = STATUS_CODE::METHOD_NOT_ALLOWED;
        }
    }

    void Request::check_content_length(std::string &value)
    {
        long long len = stoll(value);
        if (len < 0)
        {
            status_code = STATUS_CODE::BAD_REQUEST;
            content_length = 0;
        }
        else
            content_length = len;
    }

    void Request::register_header(const std::string &name, std::string value)
    {
        if (headers.find(name) != headers.end())
        {
            status_code = STATUS_CODE::BAD_REQUEST;
            return;
        }

        while (isspace(value[value.size() - 1]))
            value.pop_back();

        if (name == "Host")
            host = value;
        else if (name == "Content-Length")
            check_content_length(value);
        else if (name == "Authorization")
            auth = value;
        headers[name] = value;
    }

    void Request::parse_headers(std::stringstream ss)
    {
        std::string line;
        while (getline(ss, line) && line != "")
        {
            std::string name;
            std::string value;
            if (line[line.size() - 1] == '\r')
                line.erase(line.size() - 1);
            if (line.find(":") != std::string::npos)
            {
                auto pos = line.find(":");
                name = line.substr(0, pos++);

                while (isspace(line[pos]))
                    pos++;
                value = line.substr(pos, line.size() - pos);

                register_header(name, value);
            }
        }
    }

    void Request::parse_protocol_version(std::string http_version)
    {
        try
        {
            if (http_version.find("/") == std::string::npos
                || http_version.substr(0, http_version.find("/")) != "HTTP")
                status_code = STATUS_CODE::BAD_REQUEST;
            std::string version =
                http_version
                    .substr(http_version.find("/") + 1, http_version.size() - 1)
                    .c_str();
            if (version.size() != 3
                || !(isdigit(version[0]) && version[1] == '.'
                     && isdigit(version[2])))
                status_code = STATUS_CODE::BAD_REQUEST;
            else if (std::stof(version) < 1.1)
                status_code = STATUS_CODE::UPGRADE_REQUIRED;
        }
        catch (const std::exception &e)
        {
            status_code = STATUS_CODE::BAD_REQUEST;
        }
    }

    void Request::parse_request(const std::string &message)
    {
        std::stringstream ss(message);
        std::string method_string;
        std::string http_version;
        ss >> method_string;
        ss >> uri;
        ss >> http_version;

        parse_method(method_string);

        parse_protocol_version(http_version);

        parse_headers(std::move(ss));
        body = message.substr(message.find("\r\n\r\n") + 4);
        if (body != "" && content_length == 0)
            status_code = STATUS_CODE::BAD_REQUEST;

        build_uri();
        if (uri[0] != '/')
            status_code = STATUS_CODE::BAD_REQUEST;
    }

    bool Request::is_good()
    {
        return status_code == STATUS_CODE::OK
            || status_code == STATUS_CODE::UNAUTHORIZED;
    }

    void Request::pretty_print()
    {
#ifdef _DEBUG
        std::string methods[NB_OF_METHODS + 1] = {
            "GET",     "HEAD",    "POST",  "PUT",   "DELETE",
            "CONNECT", "OPTIONS", "TRACE", "PATCH", "ERR"
        };
        std::cout << methods[static_cast<int>(method)] << ' ' << uri << ' '
                  << "HTTP/1.1\n";

        for (auto h : headers)
            std::cout << h.first << ": " << h.second << '\n';

        if (body != "")
            std::cout << '\n' << body << '\n';
#endif
    }

} // namespace http