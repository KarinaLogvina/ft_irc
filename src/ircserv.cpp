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
	struct addrinfo	*serv;
	int				error;

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

	struct pollfd	fds[OPEN_MAX];
	int				fd_count;
	int				i;

	fds[0].fd = serv_socket;
	fds[0].events = POLLIN;
	fd_count = 1;
	while (1)
	{
		if (poll(fds, fd_count, -1) == -1)
		{
			perror("Error: poll");
			return -1;
		}
		i = 0;
		while (i < fd_count)
		{
			if (fds[i].revents != 0)
			{
				if (fds[i].fd == serv_socket)
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
					fds[fd_count].fd = client_socket;
					fds[fd_count].events = POLLIN;
					fd_count++;
					std::cout << "Client " << client_socket << " connected" \
						<< std::endl;
				}
				else
				{
					char	buffer[BUFFER_LENGTH];
					int		bytes_received;
				
					bytes_received = recv(fds[i].fd, \
							buffer, BUFFER_LENGTH - 1, 0);
					if (bytes_received == -1)
					{
						perror("Error: recv");
						return -1;
					}
					if (bytes_received == 0)
					{
						close(fds[i].fd);
						std::cout << "Client " << fds[i].fd \
							<< " closed connection" << std::endl;
						fds[i].fd = fds[fd_count - 1].fd;
						fd_count--;
					}
					else
					{
						buffer[bytes_received] = '\0';
						std::cout << "Received message: " << buffer \
							<< " from client " << fds[i].fd << std::endl;
					}
				}
			}
			i++;
		}
	}

	close(serv_socket);

	return 0;
}
