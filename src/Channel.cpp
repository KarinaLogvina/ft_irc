#include "../includes/Channel.hpp"

Channel::Channel(){
  this->topic = 0;
  this->key = 0;
  this->limit = 0;
  this->name = "";
  this->password = "";
  this->createdAt = "";
  this->createdTime = "";
  this->topicName = "";
  this->is_invite_only = 0;
}

Channel::~Channel(){};
Channel::Channel(Channel const &src) {*this = src;}
Channel &Channel::operator=(Channel const &src){
  if (this != &src) {
    this->topic = src.topic;
    this->key = src.key;
    this->limit = src.limit;
    this->name = src.name;
    this->password = src.password;
    this->createdAt = src.createdAt;
    this->createdTime = src.creationTime;
    this->topicName = src.topicName;
    this->clients = src.clients;
    this->admins = src.admins;
    this->createdTime = src.createdTime;
    this->is_invite_only = src.is_invite_only;
  }
  return *this;
}

int Channel::GetTopic() { return this->topic}
int Channel::GetKey() { return this->key};
int Channel::GetLimit() { return this->limit};
int Channel::GetNumberOfClients() { return this->clients.size() + this->admins.size()};
std::string Channel::GetChannelName() { return this->name};
std::string Channel::GetPassword() { return this->password};
std::string Channel::GetTimestamp() { return this->creationTime};
std::string Channel::clientChannel_list() {
  std::string clientsList;
  for(size_t i = 0; i < admins.size(); i++){
    clientsList += "@" + admins[i].getNickname();
    if((i + 1) < admins.size())
      clientsList += " ";
  }
  if(clients.size())
    clientsList += " ";
  for(size_t i = 0; i < clients.size(); i++){
    clientsList += clients[i].getNickname();
    if((i + 1) < clients.size())
      clientsList += " ";
  }
  return clientsList;
}

Client *Channel::get_client(int fd){
  for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it){
    if (it->GetFd() == fd)
      return &(*it);
  }
  return NULL;
}

Client *Channel::get_admin(int fd){
  for (std::vector<Client>::iterator it = admins.begin(); it != admins.end(); ++it){
    if (it->GetFd() == fd)
      return &(*it);
  }
  return NULL;
}

int Channel::GetInvitOnly(){return this->is_invite_only;}

Client *Channel::FindClientInChannel(std::string name) {
   for (std::vector<Client>::iterator it = admins.begin(); it != admins.end(); ++it){
    if (it->getNickname() == name)
      return &(*it);
  }
    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it){
    if (it->getNickname() == name)
      return &(*it);
  }
  return NULL;
}

//---setters---
void Channel::addClient(Client newClient) {clients.push_back(newClient);}
void Channel::addAdmin(Client newAdmin) {admins.push_back(newAdmin);}
void Channel::removeClient(int fd) {
  for (std::vector<Client>::iterator it =clients.begin(); it !=clients.end(); ++it) {
    if (it->GetFd() == fd) {
      clients.erase(it);
      break;
    }
  }
}

void Channel::removeAdmin(int fd) {
  for (std::vector<Client>::iterator it =admins.begin(); it !=admins.end(); ++it) {
    if (it->GetFd() == fd) {
      admins.erase(it);
      break;
    }
  }
}

