#include "../includes/Server.hpp"

int	main(int argc, char **argv)
{
    Server server;

	if (argc != 3)
	{
		if (argc == 1)
			std::cout << "Usage: ./ircserv <port> <password>" << std::endl;
		else
			std::cout << "Error: Wrong number of arguments" << std::endl;
		return -1;
	}

    // signal(SIGINT, Server::SignalHandler); //how should we handle those signals??
    // signal(SIGQUIT, Server::SignalHandler);

	if (server.init(argv[1], argv[2]) == -1)
		return -1;
	server.start();

	return 0;
}