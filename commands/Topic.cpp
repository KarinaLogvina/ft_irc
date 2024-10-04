#include "../includes/Server.hpp"


std::string Channel::TopicTimeStamp() {
    std::time_t current = std::time(NULL);
    std::stringstream res;
    res << current;
    return res.str();
}

std::string Channel::getTopicTimeStamp() const {
    return topicTimeStamp;
}

void Channel::setChangedBy(std::string hostname) {
    this->changedBy = hostname;
}

std::string Channel::getChangedBy() const {
    return changedBy;
}

std::string Server::getTopic(std::string &input) {
	size_t pos = input.find(":");
	if (pos == std::string::npos) {
		return "";
	}
	return input.substr(pos);
}

int Server::getPositionOfColon(std::string &cmd) {
	for (int i = 0; i < (int)cmd.size(); i++)
		if (cmd[i] == ':' && (cmd[i - 1] == 32))
			return i;
	return -1;
}


int Server::Topic(std::string &command, int fd) {
    std::string clientNick = getClient(fd)->getNickname();

    if (!getClient(fd)->getIsRegistered()) {
      _sendResponse(ERR_NOTREGISTERED(clientNick), fd);
      return ERR;
    }

    if (command == "TOPIC :") {
        _sendResponse(ERR_NOTENOUGHTPARAMS(clientNick, "TOPIC"), fd);
        return ERR;
    }

    std::vector<std::string> splitted_command = split_command(command);

    if (splitted_command.size() < 2) {
        _sendResponse(ERR_NOTENOUGHTPARAMS(clientNick, "TOPIC"), fd);
        return ERR;
    }

    std::string channelName = splitted_command[1].substr(1);
    Channel* channel = GetChannel(channelName);

    if (!channel) {
        _sendResponse(ERR_NOSUCHCHANNEL(clientNick, channelName), fd);
        return ERR;
    }

    Client* client = getClient(fd);

    if (!channel->get_client(fd) && !channel->get_admin(fd)) {
        _sendResponse(ERR_NOTONCHANNEL(clientNick, channelName), fd);
        return ERR;
    }

    // Если команда состоит из двух частей, вернуть текущую тему или сообщение об отсутствии темы
    if (splitted_command.size() == 2) {
        if (channel->GetTopicName().empty()) {
            _sendResponse(RPL_NOTOPIC(clientNick, channelName), fd);
        } else {
            _sendResponse(RPL_TOPIC(clientNick, channelName, channel->GetTopicName()), fd);
            _sendResponse(RPL_TOPICWHOTIME(clientNick, channelName, channel->getChangedBy(), channel->getTopicTimeStamp()), fd);
        }
        return ERR;
    }

    // Обработка установки новой темы
    std::string newTopic;
    int pos = getPositionOfColon(command);

    if (pos == -1 || splitted_command[2][0] != ':')
        newTopic = splitted_command[2];
    else
        newTopic = command.substr(pos + 1);

    // Проверка на привилегии для установки темы
    if (channel->GetTopicRestriction() && !channel->get_admin(fd)) {
        _sendResponse(ERR_CHANOPRIVSNEEDED(getClient(fd)->getNickname(), channelName), fd);
        return ERR;
    }

    // Установка новой темы и времени
    channel->SetTopicName(newTopic);
    channel->SetTopicTimeStamp(channel->TopicTimeStamp());
    channel->setChangedBy(getClient(fd)->getHostname());

    std::string response = ":" + client->getHostname() + " TOPIC #" + channelName + " :" + newTopic + "\r\n";
    channel->sendToAll(response);
    return 0;
}
