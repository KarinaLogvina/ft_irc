#include "../includes/Server.hpp"


std::string Server::getTopicTime() {
    std::time_t current = std::time(NULL);
    std::stringstream res;
    res << current;
    return res.str();
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


void Server::Topic(std::string &command, int &fd) {
    if (command == "TOPIC :") {
        senderror(461, getClient(fd)->getNickname(), fd, " :Not enough parameters\r\n");
        return;
    }

    std::vector<std::string> splitted_command = split_command(command);

    if (splitted_command.size() < 2) {
        senderror(461, getClient(fd)->getNickname(), fd, " :Not enough parameters\r\n");
        return;
    }

    std::string channelName = splitted_command[1].substr(1);
    Channel* channel = GetChannel(channelName);

    if (!channel) {
        senderror(403, "#" + channelName, fd, " :No such channel\r\n");
        return;
    }

    Client* client = getClient(fd);

    if (!channel->get_client(fd) && !channel->get_admin(fd)) {
        senderror(442, "#" + channelName, fd, " :You're not on that channel\r\n");
        return;
    }

    // Если команда состоит из двух частей, вернуть текущую тему или сообщение об отсутствии темы
    if (splitted_command.size() == 2) {
        if (channel->GetTopicName().empty()) {
            _sendResponse(": 331 " + client->getNickname() + " #" + channelName + " :No topic is set\r\n", fd);
        } else {
            std::string topicResponse = ": 332 " + client->getNickname() + " #" + channelName + " " + channel->GetTopicName() + "\r\n";
            std::string timeResponse = ": 333 " + client->getNickname() + " #" + channelName + " " + client->getNickname() + " " + channel->GetTimestamp() + "\r\n";
            _sendResponse(topicResponse, fd);
            _sendResponse(timeResponse, fd);
        }
        return;
    }

    // Обработка установки новой темы
    std::string newTopic;
    int pos = getPositionOfColon(command);

    if (pos == -1 || splitted_command[2][0] != ':') {
        newTopic = splitted_command[2];
    } else {
        newTopic = command.substr(pos);
    }

    if (newTopic == ":") {
        senderror(331, "#" + channelName, fd, " :No topic is set\r\n");
        return;
    }

    // Проверка на привилегии для установки темы
    if (channel->GetTopicRestriction() && !channel->get_admin(fd)) {
        senderror(482, "#" + channelName, fd, " :You're Not a channel operator\r\n");
        return;
    }

    // Установка новой темы и времени
    channel->SetTopicName(newTopic);
    channel->SetTime(getTopicTimestamp());

    std::string response = ":" + client->getNickname() + "!" + client->getUserName() + "@localhost TOPIC #" + channelName + " " + newTopic + "\r\n";
    channel->sendToAll(response);
}
