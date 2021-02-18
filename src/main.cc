#include <cstring>
#include <iostream>

#include "config/config.hh"
#include "error/not-implemented.hh"
#include "vhost/dispatcher.hh"
#include "vhost/vhost-factory.hh"

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
    http::Dispatcher dispatcher;
    for (auto v : config.vhosts)
        dispatcher.add_vhost(http::VHostFactory::Create(v));

    std::cout << "Listening...\n";

    return 0;
}
