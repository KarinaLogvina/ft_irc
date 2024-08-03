

#ifndef FT_IRC_SERVER_HPP
#define FT_IRC_SERVER_HPP

#include "Client.hpp"
#include "Channel.hpp"
#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sstream>
#include <fcntl.h>
#include "poll.h"
#include <cstring>

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
    void Invite(std::string &cmd, int &fd);
    void Topic(std::string &command);
    void ParseCommand(std::string &command, int &fd);
};

#endif //FT_IRC_SERVER_HPP
