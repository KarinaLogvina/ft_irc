#include "../includes/Server.hpp"

// Converts mode to append based on the last occurrence of '+' or '-'
std::string Server::modeToAppend(const std::string& chain, char operation, char mode) {
    char last = '\0';
    for (std::string::const_iterator it = chain.begin(); it != chain.end(); ++it) {
        char ch = *it;
        if (ch == '+' || ch == '-') {
            last = ch;
        }
    }

    std::stringstream ss;
    if (last != operation) {
        ss << operation;
    }
    ss << mode;
    return ss.str();
}

// Parses a command into name, modeSet, and parameters
void Server::parseCommand(const std::string& command, std::string& name, std::string& modeSet, std::string& params) {
    std::istringstream stream(command);
    stream >> name >> modeSet;
    std::getline(stream >> std::ws, params);
}

// Splits parameters by comma and removes leading colon if present
std::vector<std::string> Server::splitParams(const std::string& params) {
    std::string cleanParams = (params[0] == ':') ? params.substr(1) : params;
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream stream(cleanParams);

    while (std::getline(stream, token, ',')) {
        tokens.push_back(token);
    }
    return tokens;
}

// Handles the 'invite only' mode for a channel
std::string Server::inviteOnly(Channel* channel, char operation, const std::string& chain) {
    bool isInviteOnly = channel->getModeAtindex(0);
    std::string param;

    if (operation == '+' && !isInviteOnly) {
        channel->setModeAtindex(0, true);
        channel->SetInvitOnly(true);
        param = modeToAppend(chain, operation, 'i');
    } else if (operation == '-' && isInviteOnly) {
        channel->setModeAtindex(0, false);
        channel->SetInvitOnly(false);
        param = modeToAppend(chain, operation, 'i');
    }

    return param;
}

// Handles the 'topic restriction' mode for a channel
std::string Server::topicRestriction(Channel* channel, char operation, const std::string& chain) {
    bool isTopicRestricted = channel->getModeAtindex(1);

    if ((operation == '+' && !isTopicRestricted) || (operation == '-' && isTopicRestricted)) {
        bool newValue = (operation == '+');
        channel->setModeAtindex(1, newValue);
        channel->setTopicRestriction(newValue);
        return modeToAppend(chain, operation, 't');
    }
    return "";
}

// Handles the 'password' mode for a channel
std::string Server::passwordMode(std::vector<std::string> tokens, Channel* channel, size_t& pos, char operation, int fd, std::stringstream& mode_chain, std::string& arguments) {
    if (tokens.size() <= pos) {
        _sendResponse(ERR_INVALIDMODEPARM(getClient(fd)->getNickname(), channel->GetChannelName(), "k", "<key>"), fd);
        return "";
    }

    std::string pass = tokens[pos++];

    if (!validPassword(pass)) {
        _sendResponse(ERR_INVALIDMODEPARM(getClient(fd)->getNickname(), channel->GetChannelName(), "k", "<key>"), fd);
        return "";
    }

    std::string param;
    if (operation == '+') {
        channel->setModeAtindex(2, true);
        channel->SetPassword(pass);
        if (!arguments.empty()) {
            arguments += " ";
        }
        arguments += pass;
        param = modeToAppend(mode_chain.str(), operation, 'k');
    } else if (operation == '-' && channel->getModeAtindex(2) && pass == channel->GetPassword()) {
        channel->setModeAtindex(2, false);
        channel->SetPassword("");
        param = modeToAppend(mode_chain.str(), operation, 'k');
    } else if (operation == '-') {
        _sendResponse(ERR_KEYSET(channel->GetChannelName()), fd);
    }

    return param;
}


// Handles operator privileges for a channel
std::string Server::operatorPrivilege(std::vector<std::string> tokens, Channel* channel, size_t& pos, int fd, char operation, std::string& chain, std::string& arguments) {
    if (tokens.size() <= pos) {
        _sendResponse(ERR_INVALIDMODEPARM(getClient(fd)->getNickname(),channel->GetChannelName(), "o", "operator"), fd);
        return "";
    }

    std::string user = tokens[pos++];
    if (!channel->clientInChannel(user)) {
        _sendResponse(ERR_NOSUCHNICK(getClient(fd)->getNickname(), user), fd);
        return "";
    }

    std::string param;
    if (operation == '+') {
        channel->setModeAtindex(3, true);
        if (channel->changeClientToAdmin(user)) {
            param = modeToAppend(chain, operation, 'o');
            if (!arguments.empty()) {
                arguments += " ";
            }
            arguments += user;
        }
    } else if (operation == '-' && channel->getModeAtindex(3)) {
        if (channel->changeAdminToClient(user)) {
            param = modeToAppend(chain, operation, 'o');
            if (!arguments.empty()) {
                arguments += " ";
            }
            arguments += user;
        }
    }

    return param;
}

// Validates if a password is valid
bool validPassword(const std::string& password) {
    if (password.empty()) {
        return false;
    }
    for (std::string::const_iterator it = password.begin(); it != password.end(); ++it) {
        if (!std::isalnum(*it) && *it != '_') {
            return false;
        }
    }
    return true;
}


bool Server::isvalidLimit(const std::string& limit) {
    // Manually check if all characters are digits
    for (std::string::const_iterator it = limit.begin(); it != limit.end(); ++it) {
        if (!std::isdigit(*it)) {
            return false;
        }
    }

    // Convert the string to an integer using std::atoi and check if the value is greater than 0
    return std::atoi(limit.c_str()) > 0;
}


// Handles the 'channel limit' mode for a channel

std::string Server::channelLimit(std::vector<std::string> tokens, Channel* channel, std::vector<std::string>::size_type& pos, char operation, int fd, std::string& chain, std::string& arguments) {
    if (operation == '+') {
        if (tokens.size() > pos) {
            std::string limit = tokens[pos++];
            if (!isvalidLimit(limit)) {
                _sendResponse(ERR_INVALIDMODEPARM(getClient(fd)->getNickname(), channel->GetChannelName(), "l", limit), fd);
            } else {
                channel->setModeAtindex(4, true);
                channel->SetLimit(static_cast<int>(std::atoi(limit.c_str())));  // Using std::atoi for C++98 compatibility
                if (!arguments.empty()) {
                    arguments += " ";
                }
                arguments += limit;
                return modeToAppend(chain, operation, 'l');
            }
        } else {
            _sendResponse(ERR_NEEDMOREPARAMS(getClient(fd)->getNickname(), "MODE"), fd);
        }
    } else if (operation == '-' && channel->getModeAtindex(4)) {
        channel->setModeAtindex(4, false);
        channel->SetLimit(0);
        return modeToAppend(chain, operation, 'l');
    }
    return "";
}

// Processes the mode command
int Server::Mode(std::string& command, int fd) {
    std::string channelName;
    std::string params;
    std::string modeset;
    std::stringstream mode_chain;
    std::string arguments;
    Channel* channel = NULL;
    char operation = '\0';

    arguments = "";
    mode_chain.str("");
    Client* client = getClient(fd);
    std::string::size_type found = command.find_first_not_of("MODEmode \t\v");

    if (!client->getIsRegistered()) {
      _sendResponse(ERR_NOTREGISTERED(getClient(fd)->getNickname()), fd);
      return ERR;
    }

    if (found != std::string::npos) {
        command = command.substr(found);
    } else {
        _sendResponse(ERR_NOTENOUGHTPARAMS(client->getNickname(), "MODE"), fd);
        return ERR;
    }

    parseCommand(command, channelName, modeset, params);
    std::vector<std::string> tokens = splitParams(params);

    if (channelName[0] != '#'){
        if (!GetClientByNickname(channelName))
            _sendResponse(ERR_NOSUCHNICK(client->getNickname(), channelName), client->GetFd());
        else
            _sendResponse(ERR_UMODEUNKNOWNFLAG(client->getNickname()), fd);
        return ERR;
    } else if (!(channel = GetChannel(channelName.substr(1)))) {
        _sendResponse(ERR_NOSUCHCHANNEL(client->getNickname(), channelName.substr(1)), fd);
        return ERR;
    } else if (!channel->get_client(fd) && !channel->get_admin(fd)) {
        sendChannelerror(442, client->getNickname(), channelName, client->GetFd(), " :You're not on that channel\r\n");
        return ERR;
    } else if (modeset.empty()) {
        _sendResponse(
            RPL_CHANNELMODES(client->getNickname(), channel->GetChannelName(), channel->getModes()) +
            RPL_CREATIONTIME(client->getNickname(), channel->GetChannelName(), channel->GetTimestamp()), fd);
        return ERR;
    } else if (!channel->get_admin(fd)) {
        _sendResponse(ERR_CHANOPRIVSNEEDED(getClient(fd)->getNickname(),channel->GetChannelName()), fd);
        return ERR;
    }

    std::string::size_type pos = 0;
    std::string chain;
    for (std::string::size_type i = 0; i < modeset.size(); ++i) {
        if (modeset[i] == '+' || modeset[i] == '-') {
            operation = modeset[i];
        } else {
            chain = mode_chain.str(); // Get the current chain as a string
            switch (modeset[i]) {
                case 'i': mode_chain << inviteOnly(channel, operation, chain); break;
                case 't': mode_chain << topicRestriction(channel, operation, chain); break;
                case 'k': mode_chain << passwordMode(tokens, channel, pos, operation, fd, mode_chain, arguments); break;
                case 'o': mode_chain << operatorPrivilege(tokens, channel, pos, fd, operation, chain, arguments); break;
                case 'l': mode_chain << channelLimit(tokens, channel, pos, operation, fd, chain, arguments); break;
                default: _sendResponse(ERR_UNKNOWNMODE(client->getNickname(), modeset[i]), fd); break;
            }
        }
    }

    std::string finalChain = mode_chain.str(); // Final chain for output
    if (!finalChain.empty()) {
        channel->sendToAll(RPL_CHANGEMODE(client->getHostname(), channel->GetChannelName(), finalChain, arguments));
    }
    return 0;
}
