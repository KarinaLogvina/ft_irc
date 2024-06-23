

#ifndef FT_IRC_SERVER_HPP
#define FT_IRC_SERVER_HPP

#include "Client.hpp"
#include <iostream>
#include <vector>
#include <sstream>

class Client;
class Channel;

struct addrinfo {
  sockaddr_family_t     sin_family;
  in_port_t             sin_port;
  struct  in_addr       sin_addr;
  char                  sin_zero[8];
};

struct in_addr {
  in_addr_t s_addr;
};

class Server {
  private:
    int port;
    int socket_fd;
    static bool Signal;
    std::string password;
    std::vector<Client> clients;
    std::vector<Channel> channels;
    std::vector<struct pollfd> fds;
    struct sockaddr_in add;
    struct sockaddr_in cli_add;
    struct pollfd new_cli;

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


    void CloseSocket();
    void ClearClient();
};

#endif //FT_IRC_SERVER_HPP
