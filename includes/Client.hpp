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

class Client {
  private:
    int _fd;
    int _port;

    std::string _nickname;
    std::string _username;
    std::string _hostname;
    userState  _state;
    Client(){};
    Channel* _channel;



  public:
    int GetFd(){return _fd;};

    void setFd(int fd){ _fd = fd;}
};

#endif //FT_IRC_CLIENT_HPP
