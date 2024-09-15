

#ifndef FT_IRC_SERVER_HPP
#define FT_IRC_SERVER_HPP

#include "Client.hpp"
#include "Channel.hpp"
#include <iostream>
#include <vector>
#include <ctime> 
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sstream>
#include <fcntl.h>
#include "poll.h"
#include <cstring>
#include "replies.hpp"


class Client;
class Channel;

class Server {
  private:
    int port;
    int server_socket_fd;
    static bool Signal;
    std::string password;
    std::vector<Client> clients;
    std::vector<Channel> channels;
    std::vector<struct pollfd> fds;
    struct sockaddr_in add;
    struct sockaddr_in client_add;
    struct pollfd new_client;

  public:
    Server();
    ~Server();
    Server(Server const &src);
    Server &operator=(Server const &src);

    //----getters----//

    int GetFd();
    int GetPort();
    std::string GetPassword();
    Client *GetClient(int fd);
    std::string getTopicTimestamp();
    Client *GetClientNickname(std::string nickname);
    Channel *GetChannel(std::string name);

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
    void AddNewClient(int fd);

    bool isClientRegistered(int fd);


    void CloseSocket();
    void ClearClient();

    // removers
    void removeChannel(std::string name);
    void removeChannels(int fd);
    void removeFds(int fd);
    void removeClient(int fd);

    // Signals

    void static SignalHandler(int signum);
    void	close_fds();

    void init_server(int port, std::string password);
    void set_server_socket();
    void reciveDataFromClient(int fd);

    void 		_sendResponse(std::string response, int fd);

    //---parsers

    std::vector<std::string> split_Buffer(std::string str);
    std::vector<std::string> split_command(std::string &str);

    //error_methods
    void senderror(int code, std::string clientname, int fd, std::string message);
	  void sendChannelerror(int code, std::string clientname, std::string channelname, int fd, std::string message);

    //---CMD
    void Join(std::string cmd, int fd);
    int  SplitJoin(std::vector<std::pair<std::string, std::string>>& token, std::string cmd, int fd);
    int SearchClient(const std::string &nickname);
    int HowManyChannelsClientHas(std::string nick);
    void JoinToExistingChannel(std::vector<std::pair<std::string, std::string> >&token, int i, int j, int fd);
    void JoinToNotExistingChannel(std::vector<std::pair<std::string, std::string>> &token, int i, int fd);
    void Invite(std::string &cmd, int &fd);
    std::string getTopicTime();
    std::string getTopic(std::string &input);
    int getPositionOfColon(std::string &cmd);
    void Topic(std::string &command, int &fd);
    void ParseCommand(std::string &command, int &fd);
    std::string SplitKickCommand(std::string command, std::vector<std::string> &temp, std::string &user, int fd);
    std::string SplitCmdKick(std::string cmd, std::vector<std::string> &tmp, std::string &user, int fd);
    void Kick(std::string cmd, int fd);
    void handleClientQuit(int fd, const std::string &reason, Channel &channel);
    void Quit(std::string command, int &fd);
    void CheckForChannelsAndClients(std::vector<std::string> &tmp, int fd);
    void PivMSG(std::string command, int fd);
    bool isvalidLimit(const std::string &limit);
    std::string channelLimit(std::vector<std::string> tokens, Channel *channel, std::vector<std::string>::size_type &pos, char operation, int fd, std::string &chain, std::string &arguments);
    std::string modeToAppend(const std::string &chain, char operation, char mode);
    void parseCommand(const std::string &command, std::string &name, std::string &modeSet, std::string &params);
    std::vector<std::string> splitParams(const std::string &params);
    std::string inviteOnly(Channel *channel, char operation, const std::string &chain);
    std::string topicRestriction(Channel *channel, char operation, const std::string &chain);
    std::string passwordMode(std::vector<std::string> tokens, Channel *channel, size_t &pos, char operation, int fd, std::stringstream &mode_chain, std::string &arguments);
    std::string operatorPrivilege(std::vector<std::string> tokens, Channel *channel, size_t &pos, int fd, char operation, std::string &chain, std::string &arguments);
    void Mode(std::string &command, int fd);
    std::string modeToAppend(const std::stringstream& chain, char operation, char mode);

};

#endif //FT_IRC_SERVER_HPP
