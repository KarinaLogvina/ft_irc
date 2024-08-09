#include <iostream> // cout
#include <sys/socket.h> // socket
#include <netdb.h> // getaddrinfo
#include <unistd.h> // close
#include <poll.h> // poll

#define BUFFER_LENGTH 512

int	main(int argc, char **argv)
{
	if (argc == 1)
	{
		std::cout << "Usage: ./ircserv <port> <password>" << std::endl;
		return -1;
	}
	if (argc != 3)
	{
		std::cout << "Error: Wrong number of arguments" << std::endl;
		return -1;
	}

	char	*port;
	char	*password;

	port = argv[1];
	password = argv[2];

	(void)password;

	struct addrinfo	hints;
	int				error;
	struct addrinfo	*serv;

	memset(&hints, 0, sizeof hints);
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	error = getaddrinfo(NULL, port, &hints, &serv);
	if (error)
	{
		std::cout << "Error: getaddrinfo: " << gai_strerror(error) << std::endl;
		return -1;
	}
	
	int serv_socket;

	serv_socket = socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol);
	if (serv_socket == -1)
	{
		perror("Error: socket");
		freeaddrinfo(serv);
		return -1;
	}

	if (bind(serv_socket, serv->ai_addr, serv->ai_addrlen) == -1)
	{
		perror("Error: bind");
		freeaddrinfo(serv);
		return -1;
	}
	
	freeaddrinfo(serv);

	listen(serv_socket, 10);
	std::cout << "Listening..." << std::endl;

	std::vector<struct pollfd> poll_fds;
	struct pollfd serv_pollfd;
	serv_pollfd.fd = serv_socket;  // Server socket file descriptor
	serv_pollfd.events = POLLIN;   // Monitor for incoming data (readable)
	poll_fds.push_back(serv_pollfd);  // Add to the poll_fds vector

	int poll_return;
	int timeout = 100000; //100 seconds

	while (1)
	{
		if ((poll_return = poll(poll_fds.data(), poll_fds.size(), timeout)) == -1)
		{
			perror("Error: poll");
			return -1;
		}
		if (!poll_return)
		{
			std::cout << "Error: server timeout" << std::endl;
			return -1;
		}
	
		for (size_t i = 0; i < poll_fds.size(); i++)
		{
			if (poll_fds[i].revents & POLLIN)
			{
				if (poll_fds[i].fd == serv_socket)
				{
					int				client_socket;
					struct sockaddr client_addr;
					socklen_t 		client_addrlen;
				
					client_socket = accept(serv_socket, \
							&client_addr, &client_addrlen);
					if (client_socket == -1)
					{
						perror("Error: accept");
						return -1;
					}
					struct pollfd client_pollfd;
					client_pollfd.fd = client_socket;
					client_pollfd.events = POLLIN;
					poll_fds.push_back(client_pollfd);
					std::cout << "[C1] Client " << client_socket << " connected" \
						<< std::endl;
				}
				else
				{
					char	buffer[BUFFER_LENGTH];
					int		bytes_received;
				
					bytes_received = recv(poll_fds[i].fd, buffer, sizeof buffer, 0);
					if (bytes_received == -1)
					{
						perror("Error: recv");
						return -1;
					}
					if (bytes_received == 0)
					{
						close(poll_fds[i].fd);
						poll_fds.erase(poll_fds.begin() + i);
						i--;
						std::cout << "[D] Client " << poll_fds[i].fd << " closed connection" << std::endl;
					}
					else
					{
						buffer[bytes_received] = '\0';
						std::cout << "[M] Received message: " << buffer \
							<< "	from client " << poll_fds[i].fd << std::endl;
					}
				}
			}
		}
	}
	close(serv_socket);

	return 0;
}
