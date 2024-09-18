#include "../includes/Client.hpp"
#include "Client.hpp"

Client::Client() {
    this->fd = -1;
    this->buffer = "";
    this->nickname = "";
    this->username = "";
    this->is_logged_in = false;
    this->is_registered = false;
    this->is_operator = false;
    this->ipadd = "";
}

Client::Client(int fd)
	: fd(fd) {};

Client::Client(int fd, std::string nickname, std::string username)
	: fd(fd), nickname(nickname), username(username) {};

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
        this->is_operator = src.is_operator;
    }
    return *this;
};

Client::Client(Client const &src) {*this = src;};

//---getters---
int Client::GetFd() {return this->fd;};
bool Client::getIsRegistered() {return this->is_registered;};
bool Client::getIsLoggedIn() {return this->is_logged_in;};
std::string Client::getNickname()
{
	if (this->nickname.empty())
		return "*";
	return this->nickname;
};
std::string Client::getUserName() {return this->username;};
std::string Client::getHostname(){
	std::string hostname = this->getNickname() + "!" + this->getUserName() + "@localhost";
	return hostname;
}
std::string Client::getBuffer() {return buffer;}
std::string Client::getIpAdd() {return ipadd;}
bool			Client::getIsOperator() { return (this->is_operator); };

//---setters---
void Client::setFd(int fd) {this->fd = fd;}
void Client::setIpAdd(std::string ipadd) {this->ipadd = ipadd;}
void Client::SetNickName(std::string &nickname) {this->nickname = nickname;}
void Client::SetUserName(std::string &username)
{
	this->username = username;
}
void Client::SetIsLoggedIn(bool value) {this->is_logged_in = value;}
void Client::SetIsRegistered(bool value){
	this->is_registered = value;
}
void Client::setBuffer(std::string recived){this->buffer += recived;}
void Client::setIsOperator(bool value) {this->is_operator = value;};

//--utils--

void Client::clearBuffer() {buffer.clear();}

bool Client::getInviteChannel(std::string &channelName) {
    for(size_t i =0; i < this->ChannelsInvitation.size(); i++) {
        if(this->ChannelsInvitation[i] == channelName) {
            return true;
        }
    }
    return false;
}
void Client::addChannelInvite(std::string &channelName) {
  ChannelsInvitation.push_back(channelName);
}

void Client::removeChannelInvite(std::string &channelName) {
  for (size_t i = 0; i < this->ChannelsInvitation.size(); i++){
    if (this->ChannelsInvitation[i] == channelName)
    {this->ChannelsInvitation.erase(this->ChannelsInvitation.begin() + i); return;}
  }
}


