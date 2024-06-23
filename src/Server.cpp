
#include "../includes/Server.hpp"

Server::Server(){this->socket_fd = -1};
Server::~Server(){};
Server::Server(Server const &src){*this = src;};

Server& Server & Server::operator=(const Server &src) {
  if (this != &src) {
    this->port = src.port;
    this->socket_fd = src.socket_fd;
    this->password = src.password;
    this->clients = src.clients;
    this->channels = src.channels;
    this->fds = src.fds;
  }
  return *this;
}

// ---getters---
int Server::GetPort() { return this->port};
int Server::GetFd() { return this->socket_fd};

Client *Server::GetClient(int fd) {
  for (size_t i = 0; i < this->clients.size(); ++i) {
    if(this->clients[i].GetFd() == fd) {
      return &this->clients[i];
    }
  }
  return NULL;
}

Client *Server::GetClientNickname(std::string nickname) {}(int fd) {
  for (size_t i = 0; i < this->clients.size(); ++i) {
    if(this->clients[i].getNickname() == nickname) {
      return &this->clients[i];
    }
  }
  return NULL;
}

Channel *Server::GetChannel(std::string name) {
  for (size_t i = 0; i < this->channels.size(); ++i) {
    if(this->channels[i].GetName() == name) {
      return &channels[i];
    }
  }
}


// ---setters---

void Server::SetFd(int socket_fd) {this->socket_fd = socket_fd};
void Server::SetPort(int port) {this->port = port};
void Server::SetPassword(std::string password) {this->password = password};
void Server::AddClient(Client newClient) {this->clients.push_back(newClient)};
void Server::AddChannel(Channel newChannel) {this->channels.push_back(newChannel)};
void Server::AddFd(pollfd newFd) {this->fds.push_back(newFd)};


