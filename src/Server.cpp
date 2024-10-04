#include "../includes/Server.hpp"
#include <sys/fcntl.h>
#include <cstddef>
#include <ctime>
#include <cstdlib>
#include <string>
#include <sstream>  // Included for std::ostringstream
#include <algorithm> // Included for std::find_if, std::not1, std::ptr_fun

// Constructor and Destructor
Server::Server() {
    this->serv_socket = -1;
    this->createdAt = std::time(NULL);
}
Server::~Server() {}
Server::Server(Server const &src) {
    *this = src;
}

Server &Server::operator=(const Server &src) {
    if (this != &src) {
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

Client* Server::getClient(int fd) {
    for (size_t i = 0; i < this->clients.size(); ++i) {
        if (this->clients[i].GetFd() == fd)
            return &this->clients[i];
    }
    return NULL;
}

Client* Server::GetClientByNickname(std::string nickname) {
    for (size_t i = 0; i < this->clients.size(); ++i) {
        if (this->clients[i].getNickname() == nickname)
            return &this->clients[i];
    }
    return NULL;
}

Channel* Server::GetChannel(std::string name) {
    for (size_t i = 0; i < this->channels.size(); i++) {
        if (this->channels[i].GetChannelName() == name)
            return &channels[i];
    }
    return NULL;
}

time_t* Server::getCreatedAt() {
    return &this->createdAt;
}

size_t Server::getNumberOfClients() {
    return this->clients.size();
}

// --- Setters ---
void Server::SetFd(int socket_fd) {
    this->serv_socket = socket_fd;
}
void Server::SetPort(int port) {
    this->port = port;
}
void Server::SetPassword(std::string password) {
    this->password = password;
}
void Server::AddClient(Client newClient) {
    this->clients.push_back(newClient);
}
void Server::AddChannel(Channel newChannel) {
    this->channels.push_back(newChannel);
}
void Server::AddFd(pollfd newFd) {
    this->fds.push_back(newFd);
}

// Signal Handling
bool Server::Signal = false;
void Server::SignalHandler(int signum) {
    std::cout << std::endl << "Received signal " << strsignal(signum) << std::endl;
    Server::Signal = true;
}

void Server::close_fds() {
    for (size_t i = 0; i < clients.size(); i++) {
        std::cout << "Client <" << clients[i].GetFd() << "> Disconnected" << std::endl;
        close(clients[i].GetFd());
    }
    if (serv_socket != -1) {
        std::cout << "Server <" << serv_socket << "> Disconnected" << std::endl;
        close(serv_socket);
    }
}

int Server::init(char *port, std::string password) {
    struct addrinfo hints;
    struct addrinfo *serv;
    int error;
    int optval = 1;
    this->serverIp = ":127.0.0.1 ";

    memset(&hints, 0, sizeof hints);
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    error = getaddrinfo(NULL, port, &hints, &serv);
    if (error) {
        std::cout << "Error: getaddrinfo: " << gai_strerror(error) << std::endl;
        return -1;
    }

    serv_socket = socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol);
    if (serv_socket == -1) {
        herror("Error: socket");
        freeaddrinfo(serv);
        return -1;
    }
    if (setsockopt(serv_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        herror("Error: setsockopt");
        freeaddrinfo(serv);
        return -1;
    }
    if (bind(serv_socket, serv->ai_addr, serv->ai_addrlen) == -1) {
        herror("Error: bind");
        freeaddrinfo(serv);
        return -1;
    }
    freeaddrinfo(serv);

    add_fd(serv_socket);
    this->password = password;

    return 0;
}

void Server::start(void) {
    size_t i;

    if (listen(serv_socket, 128) == -1)
        this->stop("listen");
    std::cout << "Listening..." << std::endl;

    while (1) {
        // In C++98, std::vector does not have data(), so use &fds[0]
        if (poll(&fds[0], fds.size(), -1) == -1)
            this->stop("poll");
        i = 0;
        while (i < fds.size() && fds[i].revents == 0)
            i++;
        if (i >= fds.size())
            continue;
        if (fds[i].fd == serv_socket)
            accept_client();
        else
            receive_message_from_client(fds[i].fd);
    }
}

void Server::add_fd(int socket) {
    struct pollfd fd;

    fd.fd = socket;
    fd.events = POLLIN;
    fds.push_back(fd);
}

void Server::stop(std::string error) {
    std::string error_message = "Error: " + error;
    herror(error_message.c_str());
    fds.clear();
    exit(-1);
}

void Server::accept_client(void) {
    int client_socket;
    struct sockaddr client_addr;
    socklen_t client_addrlen = sizeof(client_addr);

    client_socket = accept(serv_socket, &client_addr, &client_addrlen);
    if (client_socket == -1)
        this->stop("accept");
    this->add_fd(client_socket);
    std::cout << CYAN << "Client " << getClientIdentifier(client_socket) << " connected" << RESET << std::endl;
    this->AddNewClient(client_socket);
}

void Server::receive_message_from_client(int fd) {
    char buffer[BUFFER_LENGTH];
    int bytes_received;

    memset(buffer, 0, sizeof(buffer));
    bytes_received = recv(fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received == -1)
        stop("recv");

    if (bytes_received == 0) {
        disconnect_client(fd);
        return;
    }

    buffer[bytes_received] = '\0';
    std::string buf = buffer;
    Client* client = getClient(fd);

    // Append any previously buffered data
    if (!client->getBuffer().empty()) {
        buf = client->getBuffer() + buf;
        client->clearBuffer();
    }

    // Check for complete messages
    size_t lastNewline = buf.find_last_of("\r\n");
    if (lastNewline == std::string::npos || lastNewline != buf.length() - 1) {
        // Incomplete message, buffer it
        client->setBuffer(buf);
    } else {
        // Complete message(s), parse them
        parse_messages(buf, fd);
    }
}

void Server::disconnect_client(int fd) {
    this->removeChannels(fd); // Remove client from all channels
    this->removeClient(fd);
    close(fd);
    std::cout << MAGENTA << "Client " << getClientIdentifier(fd) << " closed connection" << RESET << std::endl;
    this->removeFd(fd);
}

bool IsNotSpace(char c) {
    return !std::isspace(static_cast<unsigned char>(c));
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
          << getClientIdentifier(fd) << std::endl;
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
  std::string reply;
  std::string server("127.0.0.1 ");
  Client *client;
  this->parseTokens(buffer);
  client = this->getClient(fd);
  if (this->handleCommands(*client) == ERR) // stop parsing, command terminated with error
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
	  if (token.length() == 1)
        break;
	  client_msg.trailing = trim(token.substr(1)); // Remove leading ':'
      break;
    }
    if (!token.empty())
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

bool Server::isCommand(const std::string &token) {
    // List of supported commands
    const std::string commands[] = {"PASS", "NICK", "USER", "PING", "PRIVMSG", "JOIN", "INVITE", "KICK", "QUIT", "TOPIC", "MODE"};

    // Iterate over the commands array to check if the token matches any command
    for (int i = 0; i < 11; ++i) {
        if (commands[i] == token) {
            return true;  // Command found
        }
    }
    return false;  // Command not found
}

int Server::handleCommands(Client &client) {
    // Map of supported commands and their corresponding handler functions
    std::map<std::string, int (Server::*)(Client &)> commandMap;

    // Populate the command map with supported IRC commands
    commandMap["pass"] = &Server::handlePass;
    commandMap["nick"] = &Server::handleNick;
    commandMap["cap"] = &Server::handleCap;
    commandMap["user"] = &Server::handleUser;
    commandMap["ping"] = &Server::handlePong;
    // commandMap["PRIVMSG"] = &Server::handlePrivMsg;
    commandMap["part"] = &Server::Part;

    // Handle certain commands manually
    if (client_msg.command == "JOIN" || client_msg.command == "join")
        return Join(client_msg.raw, client.GetFd());
    if (client_msg.command == "INVITE" || client_msg.command == "invite")
        return Invite(client_msg.raw, client.GetFd());
    if (client_msg.command == "KICK" || client_msg.command == "kick")
        return Kick(client_msg.raw, client.GetFd());
    if (client_msg.command == "QUIT" || client_msg.command == "quit")
        return Quit(client_msg.raw, client.GetFd());
    if (client_msg.command == "TOPIC" || client_msg.command == "topic")
        return Topic(client_msg.raw, client.GetFd());
    if (client_msg.command == "MODE" || client_msg.command == "mode")
        return Mode(client_msg.raw, client.GetFd());
    if (client_msg.command == "PRIVMSG" || client_msg.command == "privmsg")
        return PrivMSG(client_msg.raw, client.GetFd());

    // Try to find the command in the map
    std::map<std::string, int (Server::*)(Client &)>::iterator it = commandMap.find(toLower(client_msg.command));

    // If the command is found, call the corresponding handler function
    if (it != commandMap.end()) {
        return (this->*(it->second))(client);
    }

    // If the command is unknown, send an error message to the client
    std::string reply = ":127.0.0.1 421 " + client.getNickname() + " " + client_msg.command + " :Unknown command\r\n";
    send(client.GetFd(), reply.c_str(), reply.size(), 0);

    // Return a code indicating the command was not recognized
    return UNKNOWN_CMD;
}

int Server::handlePass(Client &client) {
    std::string server = ":127.0.0.1 ";
    std::string reply;

    // Check if the client is already registered
    if (client.getIsRegistered()) {
        reply = server + ERR_ALREADYREGISTERED(client.getNickname());
        send(client.GetFd(), reply.c_str(), reply.size(), 0);
        return 0;
    }

    if (client.getIsLoggedIn())
      return ERR;

    // Check if the PASS command has at least one parameter (the password)
    if (client_msg.params.size() < 1) {
        std::cout << "Password should contain at least one parameter" << std::endl;
        reply = server + ERR_NEEDMOREPARAMS(client.getNickname(), this->client_msg.command);
        send(client.GetFd(), reply.c_str(), reply.size(), 0);
        return ERR;
    }

    // Validate the provided password
    if (client_msg.params[0] == this->password) {
        std::cout << GREEN << "Password accepted" << RESET << std::endl;
        client.SetIsLoggedIn(true);
        return 0;
    } else {
        // If the password is incorrect, disconnect the client
        std::cout << "Incorrect password" << std::endl;
        reply = server + ERR_PASSWDMISMATCH(client.getNickname());
        send(client.GetFd(), reply.c_str(), reply.size(), 0);

        // Optionally send an error and close the connection
        reply = server + "ERROR :Connection closed due to incorrect password. Try again.\r\n";
        send(client.GetFd(), reply.c_str(), reply.size(), 0);

        this->disconnect_client(client.GetFd());
        return ERR;
    }
}

int Server::handleNick(Client &client) {
    std::string server = ":127.0.0.1 ";
	std::string reply;

    // Ensure the client is logged in
    if (!client.getIsLoggedIn()) {
        std::cout << "Client is not logged in" << std::endl;
        return ERR;
    }

    // Check if the NICK command has parameters (nickname)
    if (client_msg.params.empty()) {
		_sendResponse(ERR_NONICKNAMEGIVEN(client.getNickname()), client.GetFd());
        return ERR;
    }

    // Get the new nickname from the parameters
    std::string newNickname = client_msg.params[0];

    // If the new nickname is the same as the current one, do nothing
    if (client.getNickname() == newNickname)
        return 0;

    // Check if the nickname is already in use
    if (this->nicknameExists(newNickname)) {
        _sendResponse(ERR_NICKNAMEINUSE(client.getNickname(), newNickname), client.GetFd());
        return 2;
    }

    // Check if the nickname is valid
    if (!checkNickname(newNickname)) {
		_sendResponse(ERR_ERRONEUSNICKNAME(client.getNickname(), newNickname), client.GetFd());
        return ERR;
    }

    // Change the nickname
    std::string hostname = client.getHostname();
    client.SetNickName(newNickname);

    // If the client is not yet registered, no need to notify other clients
    if (!client.getIsRegistered()) {
        if (!client.getUserName().empty()) {
            client.SetIsRegistered(true);
            welcomeClient(client);
        }
        return 0;
    }
        
    // Notify other users about the nickname change
    _sendResponse(CMD_NICK(hostname, newNickname), client.GetFd()); // Notify the client

    // Broadcast the nickname change to all connected clients
    for (size_t i = 0; i < this->clients.size(); ++i) {
        if (this->clients[i].GetFd() != client.GetFd()) {
            _sendResponse(CMD_NICK(hostname, newNickname), clients[i].GetFd());
        }
    }

    return 0;
}

int Server::handleCap(Client &client) {
    std::string reply = "\r\n";
    std::vector<std::string> &params = client_msg.params;
    if (std::find(params.begin(), params.end(), "LS") != params.end())
        reply = "CAP * LS :\r\n";
    send(client.GetFd(), reply.c_str(), reply.size(), 0);
    return 0;
}

int Server::handleUser(Client &client) {
    std::string reply = "\r\n";
    std::string server = ":127.0.0.1 ";
    
    if (!client.getIsLoggedIn()) {
        std::cout << "Client is not logged in" << std::endl;
        return ERR;
    }
    if (client_msg.params.size() < 1)
        reply = server + ERR_NEEDMOREPARAMS(client.getNickname(), this->client_msg.command);
    else if (client.getIsRegistered())
        reply = server + ERR_ALREADYREGISTERED(client.getNickname());
    else {
        client.SetUserName(client_msg.params[0]);
        if (client.getNickname() != "*") {
            welcomeClient(client);
            client.SetIsRegistered(true);
        }
        return 0;
    }
    send(client.GetFd(), reply.c_str(), reply.size(), 0);
    return 0;
}

int Server::handlePong(Client &client) {
    std::string token = "";
    if (client_msg.params.size() > 0)
        token = client_msg.params[0];
    std::string reply = ":127.0.0.1 PONG " + token + "\r\n";
    send(client.GetFd(), reply.c_str(), reply.size(), 0);
    return 0;
}

int Server::handlePrivMsg(Client &client) {
    std::string source = client.getNickname();
    std::string target = client_msg.params[0];
    Client *receiver = this->GetClientByNickname(target);
    std::string server(":127.0.0.1 "); 
    if (!receiver) {
        std::string reply = server + ERR_NOSUCHNICK(client.getNickname(), target);
        send(client.GetFd(), reply.c_str(), reply.size(), 0);
        return ERR;
    }
    std::string text = client_msg.trailing;
    std::string reply = ":" + source + " PRIVMSG " + target + " :" + text + "\r\n";
    send(receiver->GetFd(), reply.c_str(), reply.size(), 0);
    return 0;
}

void Server::printMessage(const clientMessage &msg) {
    std::cout << "Prefix: " << msg.source << "\n";
    std::cout << "Command: " << msg.command << "\n";
    std::cout << "Parameters:\n";
    for (std::vector<std::string>::const_iterator it = msg.params.begin(); it != msg.params.end(); it++) {
        std::cout << "  " << *it << "\n";
    }
    std::cout << "Trailing: " << msg.trailing << "\n";
}

Client &Server::AddNewClient(int fd) {
    Client new_client;
    new_client.setFd(fd);
    this->clients.push_back(new_client);
    return clients.back();
}

bool Server::nicknameExists(std::string nickname) {
    for (size_t i = 0; i < this->clients.size(); i++) {
        if (toLower(clients[i].getNickname()) == toLower(nickname))
            return true;
    }
    return false;
}

bool Server::checkNickname(std::string nickname) {
    if (nickname.find_first_of(" ,*?!@.") != std::string::npos)
        return false;
    if (nickname.find_first_of(" :#$&~+") == 0)
        return false;
    if (!containsOnlyASCII(nickname))
        return false;
    return true;
}

void Server::welcomeClient(Client &client) {
    std::string reply;
    std::string server = ":127.0.0.1 ";
    size_t num_clients = this->getNumberOfClients();

    // Convert num_clients to string using ostringstream (C++98 compatible)
    std::ostringstream oss;
    oss << num_clients;
    std::string num_clients_str = oss.str();

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
    reply = server + RPL_LUSERCLIENT(client.getNickname(), num_clients_str);
    send(client.GetFd(), reply.c_str(), reply.size(), 0);
    reply = server + RPL_LUSERME(client.getNickname(), num_clients_str);
    send(client.GetFd(), reply.c_str(), reply.size(), 0);
    reply = server + RPL_MOTDSTART(client.getNickname());
    send(client.GetFd(), reply.c_str(), reply.size(), 0);
    reply = server + RPL_MOTD(client.getNickname());
    send(client.GetFd(), reply.c_str(), reply.size(), 0);
    reply = server + RPL_ENDOFMOTD(client.getNickname());
    send(client.GetFd(), reply.c_str(), reply.size(), 0);
}

void Server::removeFd(int fd) {
    for (size_t i = 0; i < fds.size(); i++) {
        if (fds[i].fd == fd) {
            fds.erase(fds.begin() + i);
            return;
        }
    }
}

std::vector<std::string> Server::split_command(std::string &command) {
    std::vector<std::string> vec;
    std::istringstream stm(command);
    std::string token;
    while (stm >> token) {
        vec.push_back(token);
        token.clear();
    }
    return vec;
}

bool Server::isClientRegistered(int fd) {
    if (!getClient(fd) || getClient(fd)->getNickname().empty() || getClient(fd)->getUserName().empty()) {
        return false;
    }
    return true;
}

void Server::senderror(int code, std::string clientname, int fd, std::string message) {
    std::stringstream stringStream;
    stringStream << ":localhost " << code << " " << clientname << message;
    std::string response = stringStream.str();
    if (send(fd, response.c_str(), response.size(), 0) == -1)
        std::cerr << "send() failed" << std::endl;
}

void Server::sendChannelerror(int code, std::string clientname, std::string channelname, int fd, std::string message) {
    std::stringstream stringStream;
    stringStream << code << " " << clientname << " " << channelname << message;
    std::string response = stringStream.str();
    if (send(fd, response.c_str(), response.size(), 0) == -1)
        std::cerr << "send() failed" << std::endl;
}

void Server::_sendResponse(const std::string& response, int fd) {
    if (send(fd, response.c_str(), response.size(), 0) == -1) {
        std::cerr << "Response send() failed" << std::endl;
    }
}

// Removers 
void Server::removeClient(int fd) {
    for (size_t i = 0; i < this->clients.size(); i++) {
        if (this->clients[i].GetFd() == fd) {
            this->clients.erase(this->clients.begin() + i); 
            return;
        }
    }
}

void Server::removeChannel(std::string name) {
    for (size_t i = 0; i < this->channels.size(); i++) {
        if (this->channels[i].GetChannelName() == name) {
            this->channels.erase(this->channels.begin() + i); 
            return;
        }
    }
}

void Server::removeChannels(int fd) {
    for (size_t i = 0; i < this->channels.size(); i++) {
        int flag = 0;
        if (channels[i].get_client(fd)) {
            channels[i].removeClient(fd); flag = 1;
        } else if (channels[i].get_admin(fd)) {
            channels[i].removeAdmin(fd); flag = 1;
        }
        if (channels[i].GetNumberOfClients() == 0) {
            channels.erase(channels.begin() + i); i--; continue;
        }
        if (flag) {
            std::string reply = ":" + getClient(fd)->getNickname() + "!~" + getClient(fd)->getUserName() + "@localhost QUIT Quit\r\n";
            channels[i].sendToAll(reply);
        }
    }
}

std::string Server::getClientIdentifier(int fd) {
    Client* client = getClient(fd);
    if (client) {
        std::string nickname = client->getNickname();
        if (!nickname.empty() && nickname != "*")
            return nickname;
    }
    // If nickname is empty or "*", return fd as string
    std::ostringstream oss;
    oss << fd;
    return oss.str();
}

