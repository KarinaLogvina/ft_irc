
#include "../includes/Server.hpp"

Server::Server(){this->server_socket_fd = -1;}
Server::~Server(){};
Server::Server(Server const &src){*this = src;}

Server & Server::operator=(const Server &src) {
  if (this != &src) {
    this->port = src.port;
    this->server_socket_fd = src.server_socket_fd;
    this->password = src.password;
    this->clients = src.clients;
    this->channels = src.channels;
    this->fds = src.fds;
  }
  return *this;
}

// ---getters---
int Server::GetPort() { return this->port;}
int Server::GetFd() { return this->server_socket_fd;}

Client *Server::GetClient(int fd) {
  for (size_t i = 0; i < this->clients.size(); ++i) {
    if(this->clients[i].GetFd() == fd) {
      return &this->clients[i];
    }
  }
  return NULL;
}

Client *Server::GetClientNickname(std::string nickname){
  for (size_t i = 0; i < this->clients.size(); ++i) {
    if(this->clients[i].getNickname() == nickname) {
      return &this->clients[i];
    }
  }
  return NULL;
}

Channel *Server::GetChannel(std::string name){
	for (size_t i = 0; i < this->channels.size(); i++){
		if (this->channels[i].GetChannelName() == name)
			return &channels[i];
	}
	return NULL;
}


// ---setters---

void Server::SetFd(int socket_fd) {this->server_socket_fd = server_socket_fd;}
void Server::SetPort(int port) {this->port = port;}
void Server::SetPassword(std::string password) {this->password = password;}
void Server::AddClient(Client newClient) {this->clients.push_back(newClient);}
void Server::AddChannel(Channel newChannel) {this->channels.push_back(newChannel);}
void Server::AddFd(pollfd newFd) {this->fds.push_back(newFd);}



bool Server::Signal = false;
void Server::SignalHandler(int signum) {
  (void)signum;
  std::cout << std::endl << "Recieved signal!" << std::endl;
  Server::Signal = true; 
}


void	Server::close_fds(){
	for(size_t i = 0; i < clients.size(); i++){
		std::cout << "Client <" << clients[i].GetFd() << "> Disconnected" << std::endl;
		close(clients[i].GetFd());
	}
	if (server_socket_fd != -1){	
		std::cout << "Server <" << server_socket_fd << "> Disconnected" << std::endl;
		close(server_socket_fd);
	}
}


void Server::init_server (int port, std::string password) {
  this->password = password;
  this->port = port;
  this->set_server_socket();

  std::cout << "Server conected" << std::endl;
  std::cout << "Waiting to accept a connection...\n";
  //todo check server signal. somethig with a poll and POLINN
  //check if we have this fd in fds. if not new_client if yes recieve some data from it.
}

void Server::set_server_socket() {
  int en = 1;
  struct sockaddr_in socket_addr;
  add.sin_family= AF_INET;
  add.sin_addr.s_addr = INADDR_ANY;
  add.sin_port = htons(port);
  server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket_fd == -1) {
    throw(std::runtime_error("failed to create socket"));
  }
  if(setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
		throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	 if (fcntl(server_socket_fd, F_SETFL, O_NONBLOCK) == -1)
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
	if (bind(server_socket_fd, (struct sockaddr *)&add, sizeof(add)) == -1)
		throw(std::runtime_error("faild to bind socket"));
	if (listen(server_socket_fd, SOMAXCONN) == -1)
		throw(std::runtime_error("listen() faild"));

  new_client.fd = server_socket_fd;
  new_client.events = POLLIN;
  new_client.revents = 0;
  fds.push_back(new_client);
}

void Server::reciveDataFromClient(int fd) {
	std::vector<std::string> cmd;
	char buff[1024];
	memset(buff, 0, sizeof(buff));
	Client *client = GetClient(fd);
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);
	if(bytes <= 0)
	{
		std::cout << "Client <" << fd << "> Disconnected" << std::endl;
		//todo reovechannels, client. and fd close fd
	}
	else
	{ 
		client->setBuffer(buff);
		if(client->getBuffer().find_first_of("\r\n") == std::string::npos)
			return;
		cmd = split_Buffer(client->getBuffer());
  }
}


std::vector<std::string> Server::split_Buffer(std::string str)
{
	std::vector<std::string> vec;
	std::istringstream stm(str);
	std::string line;
	while(std::getline(stm, line))
	{
		size_t pos = line.find_first_of("\r\n");
		if(pos != std::string::npos)
			line = line.substr(0, pos);
		vec.push_back(line);
	}
	return vec;
}


std::vector<std::string> Server::split_command (std::string &command) {
	std::vector<std::string> vec;
	std::istringstream stm(command);
	std::string token;
	while(stm >> token)
	{
		vec.push_back(token);
		token.clear();
	}
	return vec;
}


void Server::senderror(int code, std::string clientname, int fd, std::string message) {
	std::stringstream stringStream;
	stringStream << ":localhost " << code << " " << clientname << message;
	std::string response = stringStream.str();
	if(send(fd, response.c_str(), response.size(),0) == -1)
		std::cerr << "send() faild" << std::endl;

};

void Server::sendChannelerror(int code, std::string clientname, std::string channelname, int fd, std::string message) {
		std::stringstream stringStream;
	stringStream << ":localhost " << code << " " << clientname << channelname << message;
	std::string response = stringStream.str();
	if(send(fd, response.c_str(), response.size(),0) == -1)
		std::cerr << "send() faild" << std::endl;
}

void Server::_sendResponse(std::string response, int fd)
{
	if(send(fd, response.c_str(), response.size(), 0) == -1)
		std::cerr << "Response send() faild" << std::endl;
}