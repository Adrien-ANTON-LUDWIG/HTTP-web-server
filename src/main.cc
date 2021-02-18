#include <cstring>
#include <iostream>

#include "config/config.hh"
#include "error/not-implemented.hh"
#include "events/listener.hh"
#include "misc/addrinfo/addrinfo.hh"
#include "socket/default-socket.hh"
#include "vhost/dispatcher.hh"
#include "vhost/vhost-factory.hh"

http::Dispatcher dispatcher;

static http::ListenerEW *create_and_bind(http::shared_vhost x)
{
    auto port = x->conf_get().port;
    std::string host = x->conf_get().ip;

    auto hint = misc::AddrInfoHint();
    hint.family(AF_UNSPEC);
    hint.socktype(SOCK_STREAM);

    auto addrinfo =
        misc::getaddrinfo(host.c_str(), std::to_string(port).c_str(), hint);

    http::DefaultSocket sfd;
    for (auto rp : addrinfo)
    {
        try
        {
            sfd = http::DefaultSocket(rp.ai_family, rp.ai_socktype,
                                      rp.ai_protocol);
            sfd.bind(rp.ai_addr, rp.ai_addrlen);
            break;
        }
        catch (const std::exception &)
        {
            continue;
        }
    }
    if (sfd.fd_get().get()->fd_ == -1)
    {
        std::cerr << "Could not bind to any interface\n";
        exit(0);
    }

    http::shared_socket sock = std::shared_ptr<http::Socket>(&sfd);
    sock.get()->listen(5);
    return new http::ListenerEW(sock);
}

static void start_server()
{
    for (auto x : dispatcher)
    {
        auto lew = create_and_bind(x);
        (void)(lew);
    }
}

int main(int argc, char *argv[])
{
    std::cout << "Let's go !\n";

    if (argc == 1 || argc > 3 || (argc == 3 && strcmp(argv[1], "-t")))
    {
        std::cout << "Usage : ./spider [-t] <config_file>\n";
        return 1;
    }

    if (argc == 3 && !strcmp(argv[1], "-t"))
    {
        std::string path(argv[2]);
        http::parse_configuration(path);
        return 0;
    }

    std::string path(argv[1]);

    auto config = http::parse_configuration(path);

    for (auto v : config.vhosts)
        dispatcher.add_vhost(http::VHostFactory::Create(v));

    for (auto v : dispatcher)
        std::cout << "Vhost ip = " << v->conf_get().ip << '\n';

    start_server();

    return 0;
}
