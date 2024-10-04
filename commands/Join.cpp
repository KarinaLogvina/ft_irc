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

    // Handle channel names with special characters by reading until the end of the command
    size_t pos = channelStr.find(' ');
    if (pos != std::string::npos) {
        // There's a space, so the actual channel names might include special characters
        std::string channelPart = channelStr.substr(0, pos);
        std::string remainingCmd = channelStr.substr(pos + 1);
        
        // Handle channel part
        std::istringstream channelStream(channelPart);
        while (std::getline(channelStream, buffer, ',')) {
            token.push_back(std::make_pair(buffer, ""));
        }

        // Handle remaining part of the command (which includes passwords if present)
        std::istringstream remainingStream(remainingCmd);
        std::getline(remainingStream, passwordStr, ' ');  // Passwords are space-separated

    } else {
        // There's no space, so we assume no special characters are involved
        std::istringstream channelStream(channelStr);
        while (std::getline(channelStream, buffer, ',')) {
            token.push_back(std::make_pair(buffer, ""));
        }
    }

    // Split passwords and assign to corresponding channels
    if (!passwordStr.empty()) {
        std::istringstream passwordStream(passwordStr);
        size_t index = 0;
        while (std::getline(passwordStream, buffer, ',') && index < token.size()) {
            token[index].second = buffer;
            index++;
        }
    }

    // Remove empty channel names
    std::vector<std::pair<std::string, std::string> > nonEmptyTokens;
    std::vector<std::pair<std::string, std::string> >::iterator it = token.begin();
    while (it != token.end()) {
        if (!it->first.empty()) {
            nonEmptyTokens.push_back(*it);
        }
        ++it;
    }
    token.swap(nonEmptyTokens);

    // Validate channels and remove invalid ones
    it = token.begin();
    while (it != token.end()) {
        if (it->first.empty() || (it->first[0] != '#' && it->first[0] != '&')) {
            senderror(403, getClient(fd)->getNickname(), getClient(fd)->GetFd(), " :No such channel\r\n");
            it = token.erase(it);
        } else {
            it->first = it->first.substr(1);
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
    std::string clientNick = cli->getNickname();
    Channel & channel = channels[j];
    std::string channelName = channel.GetChannelName();

    if (channel.FindClientInChannel(clientNick)) {
        return;
    }
    if (HowManyChannelsClientHas(clientNick) >= 20) {
        _sendResponse(ERR_TOOMANYCHANNELS(clientNick, channelName), fd);
        return;
    }
    if (!channel.GetPassword().empty() && channel.GetPassword() != token[i].second) {
        _sendResponse(ERR_BADCHANNELKEY(clientNick, channelName), fd);
        return;
    }
    if (channel.GetInvitOnly()) {
        if (!IsInvited(cli, token[i].first, 1)) {
            _sendResponse(ERR_INVITEONLYCHAN(clientNick, channelName), fd);
            return;
        }
    }
    if (channel.GetLimit() && channel.GetNumberOfClients() >= channel.GetLimit()) {
        _sendResponse(ERR_CHANNELISFULL(clientNick, channelName), fd);
        return;
    }
    channel.addClient(*cli);

    std::string joinMsg = RPL_JOINMSG(cli->getHostname(), channelName);
    channel.sendToAll(joinMsg);

    if (channel.GetTopicName().empty()) {
        _sendResponse(RPL_NAMREPLY(clientNick, channelName, channel.clientChannel_list()) + 
                      RPL_ENDOFNAMES(clientNick, channelName), fd);
            
    } else {
        _sendResponse(RPL_TOPICIS(clientNick,  channelName, channel.GetTopicName()) + 
                      RPL_TOPICWHOTIME(clientNick, channelName, channel.getChangedBy(), channel.getTopicTimeStamp()) +
                      RPL_NAMREPLY(clientNick,  channelName, channel.clientChannel_list()) + 
                      RPL_ENDOFNAMES(clientNick,  channelName), fd);
        
    }
}


void Server::JoinToNotExistingChannel(std::vector<std::pair<std::string, std::string> >&token, int i, int fd) {
	Client * cli = getClient(fd);
    if (HowManyChannelsClientHas(getClient(fd)->getNickname()) >= 20){
        senderror(405, getClient(fd)->getNickname(), getClient(fd)->GetFd(), " :You have joined too many channels\r\n"); 
        return;
    }
    Channel newChannel;
    newChannel.SetName(token[i].first);
    newChannel.addAdmin(*cli);
    // newChannel.addClient(*cli); // DO NOT Add client to client list, because he's an admin on a new channel. We have them separated or admins are a part of users?
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
    if (!getClient(fd)->getIsRegistered()) {
      _sendResponse(ERR_NOTREGISTERED(getClient(fd)->getNickname()), fd);
      return ERR;
    }

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
        senderror(407, getClient(fd)->getNickname(), getClient(fd)->GetFd(), " :Too many channels\r\n"); // FIXME How many targets should we allow
        return ERR;
    }
    for (size_t i = 0; i < token.size(); i++) {
        bool flag = false;
        for (size_t j = 0; j < this->channels.size(); j++) {
            if (channels[j].GetChannelName() == token[i].first) {
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
