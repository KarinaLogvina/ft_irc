#include "includes/Server.hpp"

int main(int ac, char **av) {
    Server server;
    if (ac == 1) {
		std::cout << "Usage: ./ircserv <port> <password>" << std::endl;
		return -1;
	};
	if (ac != 3) {
		std::cout << "Error: Wrong number of arguments" << std::endl;
		return -1;
	};
    std::cout << "-----Server-----" << std::endl;
    try {
        signal(SIGINT, Server::SignalHandler);
        signal(SIGQUIT, Server::SignalHandler);
        server.init_server(std::atoi(av[1]), av[2]);
    } catch (const std::exception& e) {
        std::cerr << "Error" << std::endl;
    }

    std::cout << "The Server Closed!" << std::endl;
}
