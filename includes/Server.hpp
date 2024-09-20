

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
#include <cstdlib>
#include "Client.hpp"
#include "response.hpp" // IWYU pragma: export
#include "utils.hpp" // IWYU pragma: export
#include <cstring>
#include <ctime>	//std::time
#include <iostream> // IWYU pragma: export  | cout
#include <map>		// IWYU pragma: export  | map
#include <sstream> // IWYU pragma: export
#include <vector>
#include <set> // IWYU pragma: export  | set

#define BUFFER_LENGTH 512
#include "replies.hpp"


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
	std::string serverIp;
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
		std::string raw;
	} client_msg;

	void parseTokens(const std::string &message);
	void printMessage(const clientMessage &msg);
	void receive_message_from_client(int fd);
	void accept_client(void);
	void disconnect_client(int fd);
	void add_fd(int socket);
	int parse_messages(std::string commands, int i);
    int parseMessageCommand(std::string buffer, int fd);
    bool isCommand(const std::string &token);
    void welcomeClient(Client &client);

    // commands
	int handleCommands(Client &client);
	int handlePass(Client &client);
	int handleNick(Client &client);
	int handleCap(Client &client);
	int handleUser(Client &client);
	bool nicknameExists(std::string nickname);
	bool checkNickname(std::string nickname);
	int handlePong(Client &client);
	int handlePrivMsg(Client & client);
	int Part(Client& client);

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
	Client *getClient(int fd);
    std::string getTopicTimestamp();
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
    bool isClientRegistered(int fd);


	void CloseSocket();
	void ClearClient();

	// removers
	void removeChannel(std::string name);
	void removeChannels(int fd);
    std::string getClientIdentifier(int fd);
    void removeClient(int fd);
    void removeFd(int fd);

	// Signals

	void static SignalHandler(int signum);
	void close_fds();

    void init_server(int port, std::string password);
    void set_server_socket();
    void reciveDataFromClient(int fd);
    void reciveDataFromClients(int fd);

	void 		_sendResponse(const std::string &response, int fd);

	//---parsers

	std::vector<std::string> split_Buffer(std::string str);
    std::vector<std::string> split_command(std::string &str);

    //error_methods
    void senderror(int code, std::string clientname, int fd, std::string message);
	  void sendChannelerror(int code, std::string clientname, std::string channelname, int fd, std::string message);

    //---CMD
    int Join(std::string cmd, int fd);
    int  SplitJoin(std::vector<std::pair<std::string, std::string> >& token, std::string cmd, int fd);
    int SearchClient(const std::string &nickname);
    int HowManyChannelsClientHas(std::string nick);
    void JoinToExistingChannel(std::vector<std::pair<std::string, std::string> >&token, int i, int j, int fd);
    void JoinToNotExistingChannel(std::vector<std::pair<std::string, std::string> > &token, int i, int fd);
    int Invite(std::string &cmd, int fd);
    std::string getTopicTime();
	std::string TopicTimeStamp();
    std::string getTopic(std::string &input);
    int getPositionOfColon(std::string &cmd);
    int Topic(std::string &command, int fd);
    std::string SplitKickCommand(std::string command, std::vector<std::string> &temp, std::string &user, int fd);
    std::string SplitCmdKick(std::string cmd, std::vector<std::string> &tmp, std::string &user, int fd);
    int Kick(std::string cmd, int fd);
    void handleClientQuit(int fd, const std::string &reason, Channel &channel);
    int Quit(std::string command, int fd);
    int PrivMSG(std::string command, int fd);
    bool isChannelName(const std::string &name);
    int parseMessage(std::string buffer, int fd);
    bool isvalidLimit(const std::string &limit);
    std::string channelLimit(std::vector<std::string> tokens, Channel *channel, std::vector<std::string>::size_type &pos, char operation, int fd, std::string &chain, std::string &arguments);
    std::string modeToAppend(const std::string &chain, char operation, char mode);
    void parseCommand(const std::string &command, std::string &name, std::string &modeSet, std::string &params);
    std::vector<std::string> splitParams(const std::string &params);
    std::string inviteOnly(Channel *channel, char operation, const std::string &chain);
    std::string topicRestriction(Channel *channel, char operation, const std::string &chain);
    std::string passwordMode(std::vector<std::string> tokens, Channel *channel, size_t &pos, char operation, int fd, std::stringstream &mode_chain, std::string &arguments);
    std::string operatorPrivilege(std::vector<std::string> tokens, Channel *channel, size_t &pos, int fd, char operation, std::string &chain, std::string &arguments);
    int Mode(std::string &command, int fd);
    std::string modeToAppend(const std::stringstream& chain, char operation, char mode);
    std::vector<std::string> CheckForChannelsAndClients(std::vector<std::string> &tmp, int fd);
};

typedef std::vector<std::pair<std::string, std::string> > TokenList;
bool validPassword(const std::string& password);

#endif // FT_IRC_SERVER_HPP
