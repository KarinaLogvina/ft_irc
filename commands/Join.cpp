#include "../includes/Server.hpp"

/*
RPL_INVITING (341)
ERR_NEEDMOREPARAMS (461)
ERR_NOSUCHCHANNEL (403)
ERR_NOTONCHANNEL (442)
ERR_CHANOPRIVSNEEDED (482)
ERR_USERONCHANNEL (443)*/


int Server::SplitJoin(std::vector<std::pair<std::string, std::string> >& token, std::string cmd, int fd) {
    std::vector<std::string> parts;
    std::string buffer, channelStr, passwordStr;
    std::istringstream stream(cmd);

    // Split command into parts
    std::string word;
    while (stream >> word) {
        parts.push_back(word);
    }

    // Check if there are enough parts
    if (parts.size() < 2) {
        token.clear();
        return ERR;
    }

    // Remove the command itself ("JOIN")
    parts.erase(parts.begin());
    if (parts.empty()) {
        token.clear();
        return ERR;
    }

    // Extract channel and password strings
    channelStr = parts[0];
    parts.erase(parts.begin());
    if (!parts.empty()) {
        passwordStr = parts[0];
    }

    // Split channels and push to token
    std::stringstream channelStream(channelStr);
    while (std::getline(channelStream, buffer, ',')) {
        token.push_back(std::make_pair(buffer, ""));
    }

    // Split passwords and assign to corresponding channels
    if (!passwordStr.empty()) {
        std::stringstream passwordStream(passwordStr);
        size_t index = 0;
        while (std::getline(passwordStream, buffer, ',') && index < token.size()) {
            token[index].second = buffer;
            index++;
        }
    }

    // Remove empty channel names
    std::vector<std::pair<std::string, std::string> > nonEmptyTokens;
    for (std::vector<std::pair<std::string, std::string> >::const_iterator it = token.begin(); it != token.end(); ++it) {
        if (!it->first.empty()) {
            nonEmptyTokens.push_back(*it);
        }
    }
    token.swap(nonEmptyTokens);

    // Validate channels and remove invalid ones
    std::vector<std::pair<std::string, std::string> >::iterator it = token.begin();
    while (it != token.end()) {
        if (it->first.empty() || (it->first[0] != '#' && it->first[0] != '&')) {
            senderror(403, getClient(fd)->getNickname(), getClient(fd)->GetFd(), " :No such channel\r\n");
            it = token.erase(it);
        } else {
            // Keep the '#' in the channel name
            ++it;
        }
    }

    return 0;
}


bool IsInvited(Client *client, std::string channelName, int flag) {
    if (client->getInviteChannel(channelName)) {
        if (flag == 1)
            client->removeChannelInvite(channelName);
        return true;
    }
    return false;
}


void Server::JoinToExistingChannel(std::vector<std::pair<std::string, std::string> >& token, int i, int j, int fd) {
    Client *cli = getClient(fd);

    if (this->channels[j].FindClientInChannel(cli->getNickname())) {
        return;
    }
    if (HowManyChannelsClientHas(cli->getNickname()) >= 10) {
        senderror(405, cli->getNickname(), cli->GetFd(), " :You have joined too many channels\r\n");
        return;
    }
    if (!this->channels[j].GetPassword().empty() && this->channels[j].GetPassword() != token[i].second) {
        if (!IsInvited(cli, token[i].first, 0)) {
            sendChannelerror(475, cli->getNickname(), token[i].first, cli->GetFd(), " :Cannot join channel (+k) - bad key\r\n");
            return;
        }
    }
    if (this->channels[j].GetInvitOnly()) {
        if (!IsInvited(cli, token[i].first, 1)) {
            _sendResponse(ERR_INVITEONLYCHAN(cli->getNickname(), channels[j].GetChannelName()), fd);
            return;
        }
    }
    if (this->channels[j].GetLimit() && this->channels[j].GetNumberOfClients() >= this->channels[j].GetLimit()) {
        senderror(471, cli->getNickname(), cli->GetFd(), " :Cannot join channel (+l)\r\n"); 
        return;
    }
    this->channels[j].addClient(*cli);

    std::string joinMsg = RPL_JOINMSG(cli->getHostname(), channels[j].GetChannelName());
    channels[j].sendToAll(joinMsg);

    if (channels[j].GetTopicName().empty()) {
        _sendResponse(
            RPL_NAMREPLY(cli->getNickname(), channels[j].GetChannelName(), channels[j].clientChannel_list()) + 
            RPL_ENDOFNAMES(cli->getNickname(), channels[j].GetChannelName()), fd);
    } else {
        _sendResponse(joinMsg + 
                      RPL_TOPICIS(cli->getNickname(),  channels[j].GetChannelName(), channels[j].GetTopicName()) + 
                      RPL_NAMREPLY(cli->getNickname(),  channels[j].GetChannelName(), channels[j].clientChannel_list()) + 
                      RPL_ENDOFNAMES(cli->getNickname(),  channels[j].GetChannelName()), fd);
        
        channels[j].sendToAll(joinMsg, fd); //BUG FIX!!! SEND TO ALL EXCEPT THE NEW ONE
    }
}


void Server::JoinToNotExistingChannel(std::vector<std::pair<std::string, std::string> >&token, int i, int fd) {
	Client * cli = getClient(fd);
    if (HowManyChannelsClientHas(getClient(fd)->getNickname()) >= 10){
        senderror(405, getClient(fd)->getNickname(), getClient(fd)->GetFd(), " :You have joined too many channels\r\n"); 
        return;
    }
    Channel newChannel;
    newChannel.SetName(token[i].first);
    newChannel.addAdmin(*cli);
    newChannel.addClient(*cli); // Add client to client list
    newChannel.setCreateiontime();
    this->channels.push_back(newChannel);

    // Notify the client that they joined the channel
    std::string joinMsg = RPL_JOINMSG(cli->getHostname(), newChannel.GetChannelName());
    _sendResponse(
        joinMsg + 
        RPL_NAMREPLY(cli->getNickname(), newChannel.GetChannelName(), newChannel.clientChannel_list()) + 
        RPL_ENDOFNAMES(cli->getNickname(), newChannel.GetChannelName()), fd);
}



bool ISClientInvited(Client *client, std::string channelName, int flag) {
    if (client->getInviteChannel(channelName)) {
        if (flag == 1) {
            client->removeChannelInvite(channelName);
        }
        return true;
    }
    return false;
}


int Server::HowManyChannelsClientHas(std::string nick) {
    int c = 0;
    for (size_t i = 0; i < this->channels.size(); i++) {
        if (this->channels[i].FindClientInChannel(nick)) {
            c++;
        }
    }
    return c;
}


int Server::Join(std::string command, int fd) {
    std::vector<std::pair<std::string, std::string> > token;
      // Check if the client's nickname is set
    if (getClient(fd)->getNickname().empty() || getClient(fd)->getNickname() == "*") {
        senderror(431, "*", fd, " :No nickname given\r\n");
        return ERR;
    }

    if (SplitJoin(token, command, fd)) {
        senderror(461, getClient(fd)->getNickname(), getClient(fd)->GetFd(), " :Not enough parameters\r\n");
        return ERR;
    }
    if (token.size() > 10) {
        senderror(407, getClient(fd)->getNickname(), getClient(fd)->GetFd(), " :Too many channels\r\n"); 
        return ERR;
    }
    for (size_t i = 0; i < token.size(); i++) {
        bool flag = false;
        for (size_t j = 0; j < this->channels.size(); j++) {
            if (this->channels[j].GetChannelName() == token[i].first) {
                JoinToExistingChannel(token, i, j, fd);
                flag = true;
                break;
            }
        }
        if (!flag)
            JoinToNotExistingChannel(token, i, fd);
    }
    return 0;
}
