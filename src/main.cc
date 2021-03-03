#include <cstring>
#include <iostream>

#include "config/config.hh"
#include "error/not-implemented.hh"
#include "events/listener.hh"
#include "events/register.hh"
#include "misc/addrinfo/addrinfo.hh"
#include "misc/readiness/readiness.hh"
#include "socket/default-socket.hh"
#include "vhost/dispatcher.hh"
#include "vhost/vhost-factory.hh"

http::Dispatcher http::dispatcher;

static http::shared_socket create_and_bind(http::shared_vhost x)
{
    auto port = x->conf_get().port;
    std::string host = x->conf_get().ip;

    auto hint = misc::AddrInfoHint();
    hint.family(AF_UNSPEC);
    hint.socktype(SOCK_STREAM);

    misc::AddrInfo addrinfo =
        misc::getaddrinfo(host.c_str(), std::to_string(port).c_str(), hint);

    http::DefaultSocket *sfd = new http::DefaultSocket();
    for (auto rp : addrinfo)
    {
        try
        {
            *sfd = http::DefaultSocket(rp.ai_family, rp.ai_socktype,
                                       rp.ai_protocol);
#ifdef _DEBUG
            sfd->setsockopt(SOL_SOCKET, SO_REUSEADDR, 1);
#endif
            sfd->bind(rp.ai_addr, rp.ai_addrlen);
            break;
        }
        catch (const std::exception &)
        {
            continue;
        }
    }
    if (sfd->fd_get().get()->fd_ == -1)
    {
        std::cerr << "Could not bind to any interface\n";
        exit(0);
    }
    sfd->set_hostname(host.c_str());

    http::shared_socket sock = std::shared_ptr<http::Socket>(sfd);
    sock.get()->listen(5);
    return sock;
}

static void start_server()
{
    for (auto x : http::dispatcher)
    {
        auto lew = create_and_bind(x);
        http::event_register.register_event<http::ListenerEW>(lew);
    }
}

int main(int argc, char *argv[])
{
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
        http::dispatcher.add_vhost(http::VHostFactory::Create(v));
#ifdef _DEBUG
    for (auto v : http::dispatcher)
        std::cout << "Vhost ip = " << v->conf_get().ip << '\n';
#endif
    start_server();

    misc::announce_spider_readiness(argv[0]);
    http::event_register.launch_loop();

    return 0;
}
