#include "../includes/Client.hpp"
#include "Client.hpp"

Client::Client() {
    this->fd = -1;
    this->buffer = "";
    this->nickname = "";
    this->username = "";
    this->is_logged_in = false;
    this->is_registered = false;
    this->ipadd = "";
}

Client::Client(std::string nickname, std::string username, int fd) : nickname(nickname), username(username), fd(fd){};
Client::~Client() {};
Client&Client::operator=(Client const &src) {
    if(this != &src) {
        this->fd = src.fd;
        this->buffer = src.buffer;
        this->nickname = src.nickname;
        this->username = src.username;
        this->is_logged_in = src.is_logged_in;
        this->is_registered = src.is_registered;
        this->ipadd = src.ipadd;
        this->ChannelsInvitation = src.ChannelsInvitation;
    }
    return *this;
};

Client::Client(Client const &src) {*this = src;};

//---getters---
int Client::GetFd() {return this->fd;};
bool Client::getIsRegistered() {return this->is_registered;};
bool Client::getIsLoggedIn() {return this->is_logged_in;};
std::string Client::getNickname() {return this->nickname;};
std::string Client::getUserName() {return this->username;};
std::string Client::getHostname(){
	std::string hostname = this->getNickname() + "!" + this->getUserName();
	return hostname;
}
std::string Client::getBuffer() {return buffer;}

//---setters---
void Client::setFd(int fd) {this->fd = fd;}
void Client::setIpAdd(std::string ipadd) {this->ipadd = ipadd;}
void Client::SetNickName(std::string &nickname) {this->nickname = nickname;}
void Client::SetIsLoggedIn(bool value) {this->is_logged_in = value;}
// void Client::SetIsRegistered(bool value) (bool value) {this->is_registered = value};
void Client::setBuffer(std::string recived){buffer += recived;}

//--utils--

void Client::clearBuffer() {buffer.clear()};

bool Client::getInviteChannel(std::string &channelName) {
    for(size_t i =0; i < this->ChannelsInvitation.size(); i++) {
        if(this->ChannelsInvitation[i] == channelName) {
            return true
        }
    }
    return false;
}
void Client::addChannelInvite(std::string &channelName) {
  ChannelsInvitation.push_back(channelName);
}

void Client::removeChannelInvite(std::string &channelName) {
  for (size_t i = 0; i < this->ChannelsInvite.size(); i++){
    if (this->ChannelsInvitation[i] == channelName)
    {this->ChannelsInvitation.erase(this->ChannelsInvitation.begin() + i); return;}
  }
}


