

#ifndef FT_IRC_SERVER_HPP
#define FT_IRC_SERVER_HPP

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>		// getaddrinfo
#include <poll.h>		// poll
#include <sys/socket.h> // socket
#include <sys/types.h>
#include <unistd.h> // close
#include "Channel.hpp"
#include "Client.hpp"
#include "response.hpp" // IWYU pragma: export
#include "utils.hpp" // IWYU pragma: export
#include <cstring>
#include <ctime>	//std::time
#include <iostream> // IWYU pragma: export  | cout
#include <map>		// IWYU pragma: export  | map
#include <sstream> // IWYU pragma: export
#include <vector>

#define BUFFER_LENGTH 512

class Client;
class Channel;

class Server
{
   private:
	int port;
	int serv_socket;
	int timeout;
	std::time_t createdAt;
	static bool Signal;
	bool logged_in;
	std::string password;
	std::vector<Client> clients;
	std::vector<Channel> channels;
	std::vector<struct pollfd> fds;
	struct pollfd serverSocket;
	struct sockaddr_in client_add;
	struct clientMessage
	{
		std::string source;
		std::string command;
		std::vector<std::string> params;
		std::string trailing;
	} client_msg;

	void parseReceivedMessage(const std::string &message);
	void printMessage(const clientMessage &msg);
	void receive_message_from_client(int i);
	void accept_client(void);
	void disconnect_client(int fd);
	void add_fd(int socket);
	void parse_messages(std::string commands, int i);
	void welcomeClient(Client &client);

	// commands
	int handleCommands(Client &client);
	int handlePass(Client &client);
	int handleNick(Client &client);
	int handleCap(Client &client);
	int handleUser(Client &client);
	bool nicknameExists();
	bool checkNickname(std::string nickname);
	int handlePong(Client &client);
	int handlePrivMsg(Client & client);

   public:
	Server();
	~Server();
	Server(Server const &src);
	Server &operator=(Server const &src);

	void stop(std::string error);
	int init(char *port, std::string password);
	void start(void);

	//----getters----//

	int GetFd();
	int GetPort();
	std::string GetPassword();
	Client *GetClient(int fd);
	Client *GetClientByNickname(std::string nickname);
	Channel *GetChannel(std::string name);
	time_t *getCreatedAt();

	size_t getNumberOfClients();

	//----setters----/
	void SetFd(int socket_fd);
	void SetPort(int port);
	void SetPassword(std::string password);
	void AddClient(Client newClient);
	void AddChannel(Channel newChannel);
	void AddFd(pollfd newFd);
	void SetUsername(std::string &username, int fd);
	void SetNickname(std::string &nickname, int fd);

	void ServerInit();
	void ServerSocket();
	Client &AddNewClient(int fd);

	void CloseSocket();
	void ClearClient();

	// removers
	void removeChannel(std::string name);
	void removeChannels(int fd);
	void removeClient(int fd);
	void removeFd(int fd);

	// Signals

	void static SignalHandler(int signum);
	void close_fds();

	void set_server_socket();
	void reciveDataFromClients();

	//---parsers

	std::vector<std::string> split_Buffer(std::string str);
	int parseMessage(std::string buffer, int fd);
};

#endif // FT_IRC_SERVER_HPP
