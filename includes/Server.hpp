

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
};

#endif //FT_IRC_SERVER_HPP
