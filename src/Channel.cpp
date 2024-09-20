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
  this->topic_restriction = false;
  char charaters[5] = {'i', 't', 'k', 'o', 'l'};
  for(int i = 0; i < 5; i++)
		modes.push_back(std::make_pair(charaters[i],false));
}

Channel::~Channel() {};
Channel::Channel(Channel const &src) {*this = src;}
Channel & Channel::operator=(Channel const &src){
  if (this != &src) {
    this->topic = src.topic;
    this->key = src.key;
    this->limit = src.limit;
    this->name = src.name;
    this->password = src.password;
    this->createdAt = src.createdAt;
    this->topic_restriction = src.topic_restriction;
    this->topicName = src.topicName;
    this->clients = src.clients;
    this->admins = src.admins;
    this->createdTime = src.createdTime;
    this->is_invite_only = src.is_invite_only;
    this->modes = src.modes;
  }
  return *this;
}

int Channel::GetTopic() { return this->topic;}
int Channel::GetKey() { return this->key;}
int Channel::GetLimit() { return this->limit;}
int Channel::GetNumberOfClients() { return this->clients.size() + this->admins.size();}
std::string Channel::GetChannelName() const { return this->name;}
std::string Channel::GetPassword() { return this->password;}
std::string Channel::GetTimestamp() { return this->createdAt;}
void Channel::SetInvitOnly(int invit_only){this->is_invite_only = invit_only;}
void Channel::SetTopic(int topic){this->topic = topic;}
void Channel::SetTopicTimeStamp(std::string time){this->topicTimeStamp = time;}
void Channel::SetKey(int key){this->key = key;}
void Channel::SetLimit(int limit){this->limit = limit;}
void Channel::SetTopicName(std::string topic_name){this->topicName = topic_name;}
void Channel::SetPassword(std::string password){this->password = password;}
void Channel::SetName(std::string& name) {
    this->name = name;
}

void Channel::setCreateiontime(){
	std::time_t _time = std::time(NULL);
	std::ostringstream oss;
	oss << _time;
	this->createdAt = std::string(oss.str());
}
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
  for (std::vector<Client>::iterator it = clients.begin(); it !=clients.end(); it++) {
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

void Channel::sendToAll(std::string rpl1, int except_fd) {
  for(size_t i = 0; i < admins.size(); i++) {
      if (admins[i].GetFd() == except_fd)
        continue;
      if(send(admins[i].GetFd(), rpl1.c_str(), rpl1.size(),0) == -1)
          std::cerr << "send() faild" << std::endl;
  }
  for(size_t i = 0; i < clients.size(); i++) {
      if (clients[i].GetFd() == except_fd)
        continue;
      if(send(clients[i].GetFd(), rpl1.c_str(), rpl1.size(),0) == -1)
          std::cerr << "send() faild" << std::endl;
  }
}

void Channel::sendToAllExcept(std::string rpl1, int fd){
  for(size_t i = 0; i < admins.size(); i++){
      if(admins[i].GetFd() != fd)
          if(send(admins[i].GetFd(), rpl1.c_str(), rpl1.size(),0) == -1)
              std::cerr << "send() faild" << std::endl;
  }
  for(size_t i = 0; i < clients.size(); i++){
      if(clients[i].GetFd() != fd)
          if(send(clients[i].GetFd(), rpl1.c_str(), rpl1.size(),0) == -1)
              std::cerr << "send() faild" << std::endl;
  }
}

bool Channel::getModeAtindex(size_t index){
  return modes[index].second;
}

void Channel::setModeAtindex(size_t index, bool mode){
  modes[index].second = mode;
}

void Channel::setTopicRestriction(bool value){
  this->topic_restriction = value;
  }

bool Channel::clientInChannel(std::string &nick) {
    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->getNickname() == nick) {
            return true;
        }
    }

    for (std::vector<Client>::iterator it = admins.begin(); it != admins.end(); ++it) {
        if (it->getNickname() == nick) {
            return true;
        }
    }

    return false; // Return false if no match was found in both lists
}

bool Channel::changeClientToAdmin(std::string& nick) {
    std::vector<Client>::iterator it;
    for (it = clients.begin(); it != clients.end(); ++it) {
        if (it->getNickname() == nick) {
            break;
        }
    }

    if (it != clients.end()) {
        admins.push_back(*it);

        clients.erase(it);
        
        return true;
    }
    return false;
}


bool Channel::changeAdminToClient(std::string& nick) {
    std::vector<Client>::iterator it;
    for (it = admins.begin(); it != admins.end(); ++it) {
        if (it->getNickname() == nick) {
            break;
        }
    }

    if (it != admins.end()) {
        clients.push_back(*it);

        admins.erase(it);
        
        return true;
    }

    return false;
}


std::string Channel::getModes() const {
    std::string mode;
    for (std::vector<std::pair<char, bool> >::const_iterator it = modes.begin(); it != modes.end(); ++it) {
        if (it->first != 'o' && it->second) {
            mode.push_back(it->first);
        }
    }
    if (!mode.empty()) {
        mode.insert(mode.begin(), '+');
    }
    return mode;
}

bool Channel::operator==(const Channel &other) {
    return this->GetChannelName() == other.GetChannelName();
}

bool Channel::isEmpty() const {
  return clients.empty() && admins.empty();
}