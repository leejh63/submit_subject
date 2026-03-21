#include <iostream>
#include <exception>

#include "Utils.hpp"
#include "Server.hpp"
#include "Signal.hpp"

int main(int argc, char** argv)
{
    if (argc != 3) {
        std::cerr << "Usage: ./ircserv <port> <password>\n";
        return 1;
    }

    int port = check_port(argv[1]);
    if (!port) {
        std::cerr << "Error: invalid port =" << argv[1] << "=\n";
        return 1;
    }

    if (!check_password(argv[2])) {
        std::cerr << "Error: invalid password =" << argv[2] << "=\n";
        return 1;
    }
    std::cout << "OK: port=" << port << " password=" << argv[2] << "\n";
    try {
        
        Signal::setup();

        Server s(port, argv[2]);
        s.initRoom();
        s.Poll();
        std::cout << "OK: socket success\n";
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}