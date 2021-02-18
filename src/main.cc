#include <iostream>

#include "config/config.hh"
#include "error/not-implemented.hh"

int main(int argc, char *argv[])
{
    std::cout << "Let's go !\n";

    if (argc == 1)
        return 1;

    auto config = http::parse_configuration(argv[1]);

    return 0;
}
