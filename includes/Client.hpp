#ifndef FT_IRC_CLIENT_HPP
#define FT_IRC_CLIENT_HPP


#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <csignal>
#include <string>

class Channel;

class Client {
  private:
    int fd;
    bool is_registered;
    bool is_logged_in;
    std::vector<std::string> ChannelsForClient;
    std::string nickname;
    std::string username;
    std::string hostname;
    std::string buffer;
    std::string ipadd;


  public:
    Client();
    ~Client();
    Client(std::string nickname, std::string username, int fd);
    Client(Client const &src);
    Client &operator=(Client const &src);
    //----getters----
    int GetFd(){return fd;};
    bool getIsRegistered();
    bool getIsLoggedIn();

    std::string getNickname();
    std::string getUserName();
    std::string getIpAdd();
    std::string getHostName();

    //----setters----
    void setFd(int fd);
    void setIpAdd(std::string ipadd);
    void SetNickName(std::string& nickname);
    void SetIsLoggedIn(bool value);
    void SetIsRegistered(bool value);
    //----utils----
    void clearBuffer();
    void addChannelInvite(std::string &channelName);
    void removeChannelInvite(std::string &channelName);
};

#endif //FT_IRC_CLIENT_HPP
