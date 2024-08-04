#include "../includes/Server.hpp"

/*
RPL_INVITING (341)
ERR_NEEDMOREPARAMS (461)
ERR_NOSUCHCHANNEL (403)
ERR_NOTONCHANNEL (442)
ERR_CHANOPRIVSNEEDED (482)
ERR_USERONCHANNEL (443)*/


int Server::SplitJoin(std::vector<std::pair<std::string, std::string>>& token, std::string cmd, int fd) {
    std::vector<std::string> parts;
    std::string buffer, channelStr, passwordStr;
    std::istringstream stream(cmd);

    // Split command into parts
    while (stream >> cmd) {
        parts.push_back(cmd);
    }

    // Check if there are enough parts
    if (parts.size() < 2) {
        token.clear();
        return 0;
    }

    // Extract channel and password strings
    parts.erase(parts.begin());
    channelStr = parts[0];
    parts.erase(parts.begin());
    if (!parts.empty()) {
        passwordStr = parts[0];
    }

    // Split channels and push to token
    std::stringstream channelStream(channelStr);
    while (std::getline(channelStream, buffer, ',')) {
        token.emplace_back(buffer, "");
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
    std::vector<std::pair<std::string, std::string>> nonEmptyTokens;
    for (const auto& t : token) {
        if (!t.first.empty()) {
            nonEmptyTokens.push_back(t);
        }
    }
    token.swap(nonEmptyTokens);

    // Validate channels and remove invalid ones
    for (auto it = token.begin(); it != token.end(); ) {
        if (it->first.empty() || it->first[0] != '#') {
            senderror(403, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), it->first, " :No such channel\r\n");
            it = token.erase(it);
        } else {
            it->first.erase(it->first.begin());
            ++it;
        }
    }

    return 1;
}


void Server::JoinToExistingChannel(std::vector<std::pair<std::string, std::string> >&token, int i, int j, int fd) {
    if(this->channels[j].FindClientInChannel(GetClient(fd)->getNickname())) {
        return;
    }
    if(SearchClient(GetClient(fd)->getNickname()) >= 10) {
        senderror(405, GetClient(fd)->getNickname(), GetClient(fd)->GetFd(), " :You have joined too many channels\r\n");
        return;
    }

}

int Server::SearchClient(std::string &nick) const {
    int count = 0;

    for (const auto& channel : this->channels) {
        if (channels.FindClientInChannel(nick)) {
            ++count;
        }
    }

    return count;
}

bool ISClientInvited(Client *client, std::string channelName, int flag) {
    if(client->getInviteChannel(channelName)) {
        if(flag == 1) {
            client->removeChannelInvite(channelName);
        }
        return true;
    }
    return false;
}

int Server::HowManyChannelsClientHas(std::string nick) {
    int c = 0;
    for(size_t i = 0; i < this->channels.size(); i++) {
        if(this->channels[i].FindClientInChannel(nick)) {
            c++;
        }
    }
    return c;
}

void Server::Join (std::string command, int fd) {
	std::vector<std::pair<std::string, std::string>> token;
    if(!SplitJoin(token, command, fd)) {
        senderror(461, GetClient(fd)->GetNickname(), GetClient(fd)->GetFd(), " :Not enough parameters\r\n");
        return;
    }
    if(token.size() > 10) {
		senderror(407, GetClient(fd)->GetNickname(), GetClient(fd)->GetFd(), " :Too many channels\r\n"); 
        return;
    }
    for(size_t i =0; i < this->token.size(); i++) {
        bool flag = false;
        for(size_t j = 0; j < this->channels[j], j++) {
            if(this->channels[i].GetChannelName() == token[i].first) {

            }
        }
    }
}