#include <iostream> // cout
#include <sys/socket.h> // socket
#include <netdb.h> // getaddrinfo
#include <unistd.h> // usleep

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
	
	int client_socket;

	client_socket = socket(serv->ai_family, serv->ai_socktype, \
			serv->ai_protocol);
	if (client_socket == -1)
	{
		perror("Error: socket");
		return -1;
	}

	if (connect(client_socket, serv->ai_addr, serv->ai_addrlen) == -1)
	{
		perror("Error: connect");
		return -1;
	}
	
	freeaddrinfo(serv);

	usleep(3000000);

	const char	*message;
	int			message_length;
	int			bytes_sent;
	
	message = "Hello world!";
	message_length = strlen(message);
	bytes_sent = send(client_socket, message, message_length, 0);
	if (bytes_sent == -1)
	{
		perror("Error: send");
		return -1;
	}
	if (bytes_sent != message_length)
	{
		std::cout << "Error: did not send the whole message" << std::endl;
		return -1;
	}
	std::cout << "Message send: "  << message << std::endl;
	
	usleep(3000000);

	message = "42";
	message_length = strlen(message);
	bytes_sent = send(client_socket, message, message_length, 0);
	if (bytes_sent == -1)
	{
		perror("Error: send");
		return -1;
	}
	if (bytes_sent != message_length)
	{
		std::cout << "Error: did not send the whole message" << std::endl;
		return -1;
	}
	std::cout << "Message send: " << message << std::endl;

	usleep(3000000);
	
	close(client_socket);

	return 0;
}
