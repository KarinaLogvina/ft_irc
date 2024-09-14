
#include "../includes/Server.hpp"
#include <sys/fcntl.h>
#include <cstddef>
#include <ctime>
#include <string>

Server::Server()
{
	this->serv_socket = -1;
	this->createdAt = std::time(NULL);
}
Server::~Server(){};
Server::Server(Server const &src)
{
	*this = src;
}

Server &Server::operator=(const Server &src)
{
	if (this != &src)
	{
		this->port = src.port;
		this->serv_socket = src.serv_socket;
		this->password = src.password;
		this->clients = src.clients;
		this->channels = src.channels;
		this->fds = src.fds;
	}
	return *this;
}

// ---getters---
int Server::GetPort() { return this->port;}
int Server::GetFd() { return this->serv_socket;}

std::string Server::getTopicTimestamp() {
	std::time_t current = std::time(NULL);  // Получаем текущее время как значение типа time_t
    std::stringstream res;                 // Создаем stringstream для построения строки
    res << current;                        // Вставляем текущее время в stringstream
    return res.str();
}

Client *Server::getClient(int fd)
{
	for (size_t i = 0; i < this->clients.size(); ++i)
	{
		if (this->clients[i].GetFd() == fd)
			return &this->clients[i];
	}
	return NULL;
}

Client *Server::GetClientByNickname(std::string nickname)
{
	for (size_t i = 0; i < this->clients.size(); ++i)
	{
		if (this->clients[i].getNickname() == nickname)
		{
			return &this->clients[i];
		}
	}
	return NULL;
}

Channel *Server::GetChannel(std::string name)
{
	for (size_t i = 0; i < this->channels.size(); i++)
	{
		if (this->channels[i].GetChannelName() == name)
			return &channels[i];
	}
	return NULL;
}

time_t *Server::getCreatedAt()
{
	return &this->createdAt;
}

size_t Server::getNumberOfClients()
{
	return this->clients.size();
}

// ---setters---

void Server::SetFd(int socket_fd)
{
	this->serv_socket = socket_fd;
}
void Server::SetPort(int port)
{
	this->port = port;
}
void Server::SetPassword(std::string password)
{
	this->password = password;
}
void Server::AddClient(Client newClient)
{
	this->clients.push_back(newClient);
}
void Server::AddChannel(Channel newChannel)
{
	this->channels.push_back(newChannel);
}
void Server::AddFd(pollfd newFd)
{
	this->fds.push_back(newFd);
}

bool Server::Signal = false;
void Server::SignalHandler(int signum)
{
	std::cout << std::endl
			  << "Recieved signal " << strsignal(signum) << std::endl;
	Server::Signal = true;
}

void Server::close_fds()
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		std::cout << "Client <" << clients[i].GetFd() << "> Disconnected"
				  << std::endl;
		close(clients[i].GetFd());
	}
	if (serv_socket != -1)
	{
		std::cout << "Server <" << serv_socket << "> Disconnected" << std::endl;
		close(serv_socket);
	}
}

int Server::init(char *port, std::string password)
{
	struct addrinfo hints;
	struct addrinfo *serv;
	int error;
	int en = 1; // разобраться
	this->serverIp = ":127.0.0.1 ";

	memset(&hints, 0, sizeof hints);
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	error = getaddrinfo(NULL, port, &hints, &serv);
	if (error)
	{
		std::cout << "Error: getaddrinfo: " << gai_strerror(error) << std::endl;
		return -1;
	}

	serv_socket = socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol);
	if (serv_socket == -1)
	{
		perror("Error: socket");
		freeaddrinfo(serv);
		return -1;
	}
	if (setsockopt(serv_socket, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1) // разобраться
	{
		perror("Error: setsockopt");
		freeaddrinfo(serv);
		return -1;
	}
	if (bind(serv_socket, serv->ai_addr, serv->ai_addrlen) == -1)
	{
		perror("Error: bind");
		freeaddrinfo(serv);
		return -1;
	}
	freeaddrinfo(serv);

	add_fd(serv_socket);
	this->password = password;

	return 0;
}

void Server::start(void)
{
	size_t i;

	if (listen(serv_socket, 128) == -1)
		this->stop("listen");
	std::cout << "Listening..." << std::endl;

	while (1)
	{
		if (poll(fds.data(), fds.size(), -1) == -1)
			this->stop("poll");
		i = 0;
		while (i < fds.size() && fds[i].revents == 0)
			i++;
		if (fds[i].fd == serv_socket)
			accept_client();
		else
			receive_message_from_client(fds[i].fd);
	}

	return;
}

void Server::add_fd(int socket)
{
	struct pollfd fd;

	fd.fd = socket;
	fd.events = POLLIN;
	fds.push_back(fd);

	return;
}

void Server::stop(std::string error)
{
	std::string error_message;

	error_message = "Error: " + error;
	perror(error_message.c_str());
	fds.clear();
	exit(-1);

	return;
}

void Server::accept_client(void)
{
	int client_socket;
	struct sockaddr client_addr;
	socklen_t client_addrlen;

	client_socket = accept(serv_socket, &client_addr, &client_addrlen);
	if (client_socket == -1)
		this->stop("accept");
	this->add_fd(client_socket);
	std::cout << "Client " << client_socket << " connected" << std::endl;
	this->AddNewClient(client_socket);

	return;
}

void Server::receive_message_from_client(int fd)
{
	char buffer[BUFFER_LENGTH];
	int bytes_received;

	memset(buffer, 0, sizeof(buffer));
	bytes_received = recv(fd, buffer, sizeof(buffer), 0);
	if (bytes_received == -1)
		stop("recv");
	buffer[bytes_received] = '\0';
	if (bytes_received == 0)
		disconnect_client(fd);
	else
	{
		std::string buf = buffer;
		Client * client = getClient(fd);

		if (buf.find("\n") == std::string::npos)
		{	std::cout << "Received partial message: \"" << buf << "\" from client "
					  << fd << std::endl;
			client->setBuffer(buf);
		}
		else 
		{
			if (!client->getBuffer().empty()) // if we have acummulated buffer
			{
				buf = client->getBuffer() + buf;
				client->clearBuffer();
			}
			parse_messages(buf, fd); //parse normal message
		}
	}
}

void Server::disconnect_client(int fd)
{
	this->removeClient(fd);
	close(fd);
	std::cout << "Client " << fd << " closed connection" << std::endl;
	this->removeFd(fd);
}

int Server::parse_messages(std::string messages, int fd) //change to stringstream
{
	std::string message;
	size_t position;

	while ( (position = messages.find_first_of("\r\n")) != std::string::npos) // changed from \r to \r\n
	{
		message = messages.substr(0, messages.find_first_of("\r\n"));
		if (message.empty())
			return ERR;
		std::cout << "Received message: \"" << message << "\" from client "
				  << fd << std::endl;
		if (this->parseMessage(message, fd) == ERR) // stop parsing
			return ERR;
		if (messages[position] == '\n') //if not an irc client, e.g. nc
			return ERR;
		messages = messages.substr(messages.find("\r") + 2);
	}

	return 0;
}

int Server::parseMessage(std::string buffer, int fd)
{
	int result;
	std::string reply;
	std::string server("127.0.0.1 ");
	Client *client;
	this->parseTokens(buffer);
	client = this->getClient(fd);
	if ((result = this->handleCommands(*client)) == UNKNOWN_CMD)
		std::cout << "This is the end of the world" << std::endl;
	if (result == ERR) // stop parsing, command terminated with error
		return ERR;
	return 0;
}

void Server::parseTokens(const std::string &message)
{
	std::istringstream iss(message);
	std::string token;
	std::string line;

	std::memset(&this->client_msg, 0,
				sizeof(this->client_msg)); // change for safe variant

	std::istringstream iss_line(message);

	client_msg.raw = message;
	// Check for prefix
	if (message[0] == ':')
	{
		std::getline(iss_line, token, ' ');
		client_msg.source = token.substr(1); // Remove leading ':'
	}

	// Get command
	std::getline(iss_line, token, ' ');
	client_msg.command = trim(token);

	// Get parameters and trailing
	bool trailingFound = false;
	while (std::getline(iss_line, token, ' '))
	{
		if (token[0] == ':')
		{
			trailingFound = true;
			client_msg.trailing = trim(token.substr(1)); // Remove leading ':'
			break;
		}
		else
			client_msg.params.push_back(trim(token));
	}

	// Get the rest of the trailing message if any
	if (trailingFound)
	{
		std::string rest;
		std::getline(iss_line, rest);
		client_msg.trailing += " " + trim(rest);
	}
}

int Server::handleCommands(Client &client)
{
	std::map<std::string, int (Server::*)(Client &)> commandMap;
	std::map<std::string, int (Server::*)(Client &)>::iterator it;
	commandMap["PASS"] = &Server::handlePass;
	commandMap["NICK"] = &Server::handleNick;
	commandMap["CAP"] = &Server::handleCap;
	commandMap["USER"] = &Server::handleUser;
	commandMap["PING"] = &Server::handlePong;
	commandMap["PRIVMSG"] = &Server::handlePrivMsg;
	if (client_msg.command == "JOIN")
		return Join(client_msg.raw, client.GetFd());
	/* Can do it with the switch? */
	it = commandMap.find(client_msg.command);
	if (it != commandMap.end())
		return (this->*(it->second))(client);
	std::string reply = serverIp + ERR_UNKNOWNCOMMAND(client.getNickname(), client_msg.command);
	send(client.GetFd(), reply.c_str(), reply.size(), 0);
	return UNKNOWN_CMD;
}

int Server::handlePass(Client &client)
{
	std::string server = ":127.0.0.1 ";
	std::string reply;
	if (client.getIsLoggedIn())
		reply = server + ERR_ALREADYREGISTERED(client.getNickname());
	else if (client_msg.params.size() < 1)
	{
		std::cout << "Password should contain at least one param" << std::endl;
		reply = server +
			ERR_NEEDMOREPARAMS(client.getNickname(), this->client_msg.command);
	}
	else if (client_msg.params[0] == password)
	{
		std::cout << "Handled a password successfully" << std::endl;
		client.SetIsLoggedIn(true);
		return 0;
	}
	else
	{
		std::cout << "Incorrect password" << std::endl;
		reply = server + ERR_PASSWDMISMATCH(client.getNickname());
		send(client.GetFd(), reply.c_str(), reply.size(), 0);
		reply = server + "ERROR :Conection closed due to incorrect password. Try again.\r\n";
		send(client.GetFd(), reply.c_str(), reply.size(), 0);
		this->disconnect_client(client.GetFd());
		return ERR;
	}
	send(client.GetFd(), reply.c_str(), reply.size(), 0);

	return 0;
}

int Server::handleNick(Client &client)
{
	std::string server = ":127.0.0.1 ";
	std::string reply;

	if (!client.getIsLoggedIn())
	{
		std::cout << "Client is not logged in" << std::endl;
		return ERR;
	}
	if (client_msg.params.empty())
		reply = server + ERR_NONICKNAMEGIVEN(client.getNickname());
	else
	{
		std::string &newNickname = client_msg.params[0];

		if (client.getNickname() == newNickname)
			return 0;
		else if (this->nicknameExists(newNickname))
			reply = server + ERR_NICKNAMEINUSE(client.getNickname(), newNickname);
		else if (!checkNickname(newNickname))
			reply = server + ERR_ERRONEUSNICKNAME(client.getNickname(), newNickname);
		else 
		{
			if (!client.getIsRegistered())
			{
				client.SetNickName(newNickname);
				return 0;
			}
			reply = ":" + client.getNickname() + CMD_NICK(client.getUserName(), newNickname);
			client.SetNickName(newNickname);
		}
	}
	
	send(client.GetFd(), reply.c_str(), reply.size(), 0);
	return 0;
}

int Server::handleCap(Client &client)
{
	std::string reply = "\r\n";
	std::vector<std::string> &params = client_msg.params;
	if (std::find(params.begin(), params.end(), "LS") != params.end())
		reply = "CAP * LS :\r\n";
	send(client.GetFd(), reply.c_str(), reply.size(), 0);
	return 0;
}

int Server::handleUser(Client &client) // check if username is valid
{
	std::string reply = "\r\n";
	std::string server = ":127.0.0.1 ";
	
	if (!client.getIsLoggedIn())
	{
		std::cout << "Client is not logged in" << std::endl;
		return ERR;
	}
	if (client_msg.params.size() < 1)
		reply = server + ERR_NEEDMOREPARAMS(client.getNickname(), this->client_msg.command);
	else if (client.getIsRegistered())
		reply = server + ERR_ALREADYREGISTERED(client.getNickname());
	else
	{
		client.SetUserName(client_msg.params[0]);
		if (client.getNickname() != "*")
		{
			welcomeClient(client);
			client.SetIsRegistered(true);
		}
		return 0;
	}
	send(client.GetFd(), reply.c_str(), reply.size(), 0);
	return 0;
}

int Server::handlePong(Client & client) //make it handle ping also
{
	std::string token = "";
	if (client_msg.params.size() > 0)
		token = client_msg.params[0];
	std::string reply = ":127.0.0.1 PONG " + token + "\r\n";
	send(client.GetFd(), reply.c_str(), reply.size(), 0);
	return 0;
}

int Server::handlePrivMsg(Client & client) // check different cases and make macro
{
	std::string source = client.getNickname();
	std::string target = client_msg.params[0];
	Client * receiver = this->GetClientByNickname(target);
	std::string server(":127.0.0.1 "); 
	if (!receiver)
	{
		std::string reply = server + ERR_NOSUCHNICK(client.getNickname(), target);
		send(client.GetFd(), reply.c_str(), reply.size(), 0);
		return ERR;
	}
	std::string text = client_msg.trailing;
	std::string reply = ":" + source + " PRIVMSG " + target + " :" + text + "\r\n";
	send(receiver->GetFd(), reply.c_str(), reply.size(), 0);
	return 0;
}


void Server::printMessage(const clientMessage &msg)
{
	std::cout << "Prefix: " << msg.source << "\n";
	std::cout << "Command: " << msg.command << "\n";
	std::cout << "Parameters:\n";
	for (std::vector<std::string>::const_iterator it = msg.params.begin();
		 it != msg.params.end(); it++)
	{
		std::cout << "  " << *it << "\n";
	}
	std::cout << "Trailing: " << msg.trailing << "\n";
}

Client &Server::AddNewClient(int fd)
{
	Client new_client;
	new_client.setFd(fd);
	this->clients.push_back(new_client);
	return clients.back();
}

bool Server::nicknameExists(std::string nickname)
{
	for (size_t i = 0; i < this->clients.size(); i++)
	{
		if (toLower(clients[i].getNickname()) == toLower(nickname))
			return true;
	}
	return false;
}

bool Server::checkNickname(std::string nickname)
{
	if (nickname.find_first_of(" ,*?!@.") != std::string::npos)
		return false;
	if (nickname.find_first_of(":#$&~+") == 0)
		return false;
	if (!containsOnlyASCII(nickname))
		return false;
	return true;
}

void Server::welcomeClient(Client &client)
{
	std::string reply;
	std::string server = ":127.0.0.1 ";
	size_t num_clients = this->getNumberOfClients();

	// how to do it with a loop?
	reply = server + RPL_WELCOME(client.getNickname(), client.getUserName());
	send(client.GetFd(), reply.c_str(), reply.size(), 0);
	reply = server + RPL_YOURHOST(client.getNickname());
	send(client.GetFd(), reply.c_str(), reply.size(), 0);
	reply = server + RPL_CREATED(client.getNickname(), std::asctime(std::localtime(this->getCreatedAt())));
	send(client.GetFd(), reply.c_str(), reply.size(), 0);
	reply = server + RPL_MYINFO(client.getNickname());
	send(client.GetFd(), reply.c_str(), reply.size(), 0);
	reply = server + RPL_ISUPPORT(client.getNickname());
	send(client.GetFd(), reply.c_str(), reply.size(), 0);
	reply = server + RPL_LUSERCLIENT(client.getNickname(), (std::ostringstream() << num_clients).str());
	send(client.GetFd(), reply.c_str(), reply.size(), 0);
	reply = server + RPL_LUSERME(client.getNickname(), (std::ostringstream() << num_clients).str());
	send(client.GetFd(), reply.c_str(), reply.size(), 0);
	reply = server + RPL_MOTDSTART(client.getNickname());
	send(client.GetFd(), reply.c_str(), reply.size(), 0);
	reply = server + RPL_MOTD(client.getNickname());
	send(client.GetFd(), reply.c_str(), reply.size(), 0);
	reply = server + RPL_ENDOFMOTD(client.getNickname());
	send(client.GetFd(), reply.c_str(), reply.size(), 0);
}

void Server::removeFd(int fd)
{
	for (size_t i = 0; i < fds.size(); i++) {
		if (fds[i].fd == fd)
			fds.erase(fds.begin() + i);
	}
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


bool Server::isClientRegistered(int fd) {
	if(!getClient(fd) || getClient(fd)->getNickname().empty() || getClient(fd)->getUserName().empty()) {
		return false;
	}
	return true;
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


void Server::_sendResponse(const std::string& response, int fd) {
    if (send(fd, response.c_str(), response.size(), 0) == -1) {
        std::cerr << "Response send() failed" << std::endl;
    }
}


void Server::ParseCommand(std::string &command, int &fd) {
	if(command.empty()) {
		return ;
	}
	std::vector<std::string> splited_command = split_command(command);
	size_t found = command.find_first_not_of(" \t\v");
	if (found != std::string::npos) {
		command = command.substr(found);
	}
   if (!isClientRegistered(fd)) {
        _sendResponse(ERR_USERNOTREGISTERED(std::string("*")), fd);
        return;
    } else {
		if (splited_command.size() && (splited_command[0] == "INVITE" || splited_command[0] == "invite")) {
			Invite(command, fd);
		} else if (splited_command.size() && (splited_command[0] == "JOIN" || splited_command[0] == "invite")) {
			Join(command, fd);
		}
	}

}

//removers 

void Server::removeClient(int fd){
	for (size_t i = 0; i < this->clients.size(); i++){
		if (this->clients[i].GetFd() == fd){
			this->clients.erase(this->clients.begin() + i); 
			return;
		}
	}
}

void Server::removeChannel(std::string name){
	for (size_t i = 0; i < this->channels.size(); i++){
		if (this->channels[i].GetChannelName() == name){
			this->channels.erase(this->channels.begin() + i); 
			return;
		}
	}
}

void Server::removeFds(int fd){
	for (size_t i = 0; i < this->fds.size(); i++){
		if (this->fds[i].fd == fd) {
			this->fds.erase(this->fds.begin() + i); 
			return;
		}
	}
}


void	Server::removeChannels(int fd){
	for (size_t i = 0; i < this->channels.size(); i++){
		int flag = 0;
		if (channels[i].get_client(fd)){
			channels[i].removeClient(fd); flag = 1;
		}
		else if (channels[i].get_admin(fd)) {
			channels[i].removeAdmin(fd); flag = 1;
		}
		if (channels[i].GetNumberOfClients() == 0) {
			channels.erase(channels.begin() + i); i--; continue;
		}
		if (flag){
			std::string reply = ":" + getClient(fd)->getNickname() + "!~" + getClient(fd)->getUserName() + "@localhost QUIT Quit\r\n";
			channels[i].sendToAll(reply);
		}
	}
}