#include "../includes/Server.hpp"

void FindPrivmessageCommand(std::string command, std::string tofind, std::string &str) {
    size_t i = 0;
    for (; i < command.size(); i++) {
        if (command[i] != ' ') {
            std::string tmp;
            for (; i < command.size() && command[i] != ' '; i++)
                tmp += command[i];
            if (tmp == tofind) break;
            else tmp.clear();
        }
    }
    if (i < command.size()) str = command.substr(i);
    i = 0;
    for (; i < str.size() && str[i] == ' '; i++);
    str = str.substr(i);
}

std::string ExtractCommandAndTarget(std::string &command, std::vector<std::string> &tmp) {
    std::stringstream ss(command);
    std::string str, message;
    int count = 2;
    while (ss >> str && count--)
        tmp.push_back(str);
    if (tmp.size() != 2) return std::string("");
    FindPrivmessageCommand(command, tmp[1], message);
    return message;
}

std::string ParsePrivmsgTargetsAndMessage(std::string cmd, std::vector<std::string> &tmp) {
    std::string str = ExtractCommandAndTarget(cmd, tmp);
    if (tmp.size() != 2) {
        tmp.clear();
        return std::string("");
    }
    tmp.erase(tmp.begin());
    std::string str1 = tmp[0];
    std::string str2;
    tmp.clear();
    for (size_t i = 0; i < str1.size(); i++) {
        //split the first string by ',' to get the channels names
        if (str1[i] == ',') {
            tmp.push_back(str2);
            str2.clear();
        } else {
            str2 += str1[i];
        }
    }
    tmp.push_back(str2);
    for (size_t i = 0; i < tmp.size(); i++) {
        //erase the empty strings
        if (tmp[i].empty()) tmp.erase(tmp.begin() + i--);
    }
    if (str[0] == ':') {
        str.erase(str.begin());
    } else {
        //shrink to the first space
        for (size_t i = 0; i < str.size(); i++) {
            if (str[i] == ' ') {
                str = str.substr(0, i);
                break;
            }
        }
    }
    return str;
}


void Server::CheckForChannelsAndClients(std::vector<std::string> &tmp, int fd) {
    for (size_t i = 0; i < tmp.size(); i++) {
        if (tmp[i][0] == '#') {
            tmp[i].erase(tmp[i].begin());
            if (!GetChannel(tmp[i])) {
                //ERR_NOSUCHNICK (401) // if the channel doesn't exist
                senderror(401, "#" + tmp[i], getClient(fd)->GetFd(), " :No such nick/channel\r\n");
                tmp.erase(tmp.begin() + i);
                i--;
            } else if (!GetChannel(tmp[i])->FindClientInChannel(getClient(fd)->getNickname())) {
                //ERR_CANNOTSENDTOCHAN (404) // if the client is not in the channel
                sendChannelerror(404, getClient(fd)->getNickname(), "#" + tmp[i], getClient(fd)->GetFd(), " :Cannot send to channel\r\n");
                tmp.erase(tmp.begin() + i);
                i--;
            } else {
                tmp[i] = "#" + tmp[i];
            }
        } else {
            if (!GetClientByNickname(tmp[i])) {
                //ERR_NOSUCHNICK (401) // if the client doesn't exist
                senderror(401, tmp[i], getClient(fd)->GetFd(), " :No such nick/channel\r\n");
                tmp.erase(tmp.begin() + i);
                i--;
            }
        }
    }
}


void Server::PivMSG(std::string cmd, int fd) {
    std::vector<std::string> targets;
    std::string message = ParsePrivmsgTargetsAndMessage(cmd, targets);

    // Проверка на наличие получателей
    if (targets.empty()) {
        senderror(411, getClient(fd)->getNickname(), getClient(fd)->GetFd(), " :No recipient given (PRIVMSG)\r\n");
        return;
    }

    // Проверка на наличие текста сообщения
    if (message.empty()) {
        senderror(412, getClient(fd)->getNickname(), getClient(fd)->GetFd(), " :No text to send\r\n");
        return;
    }

    // Проверка на слишком большое количество получателей
    if (targets.size() > 10) {
        senderror(407, getClient(fd)->getNickname(), getClient(fd)->GetFd(), " :Too many recipients\r\n");
        return;
    }

    CheckForChannelsAndClients(targets, fd); // Проверка существования каналов и клиентов

    // Отправка сообщения получателям
    for (std::vector<std::string>::iterator target = targets.begin(); target != targets.end(); target++) {
        std::string response = ":" + getClient(fd)->getNickname() + "!~" + getClient(fd)->getUserName() + "@localhost PRIVMSG " + *target + " :" + message + "\r\n";
        if ((*target)[0] == '#') {
            std::string channelName = (*target).substr(1); // Удаляем символ '#'
            GetChannel(channelName)->sendToAllExcept(response, fd);
        } else {
            _sendResponse(response, GetClientByNickname(*target)->GetFd());
        }
    }
}
