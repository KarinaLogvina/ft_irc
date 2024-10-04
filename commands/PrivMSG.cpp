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


std::vector<std::string> Server::CheckForChannelsAndClients(std::vector<std::string> &tmp, int fd) {
    std::vector<std::string> targets;
    std::set<std::string> unique;
    for (std::vector<std::string>::const_iterator target = tmp.begin(); target != tmp.end(); ++target) {
        if (unique.find(*target) != unique.end())
            continue;
        if ((*target)[0] == '#' && !GetChannel((*target).substr(1))) {
            _sendResponse(ERR_NOSUCHNICK(getClient(fd)->getNickname(), (*target)),  fd);
        } else if ((*target)[0] != '#' && !GetClientByNickname(*target)) {
            _sendResponse(ERR_NOSUCHNICK(getClient(fd)->getNickname(), *target),  fd);
        } else {
            targets.push_back(*target);
        }
        unique.insert(*target);
        // std::cout << *target << std::endl;
    }
    return targets;
}


bool Server::isChannelName(const std::string& name) {
    if (name.empty())
        return false;
    char prefix = name[0];
    return (prefix == '#' || prefix == '&');
}

int Server::PrivMSG(std::string cmd, int fd) {
    std::vector<std::string> targets;
    std::string message = ParsePrivmsgTargetsAndMessage(cmd, targets);
    Client & cli = *getClient(fd);
    if (!cli.getIsRegistered()) {
      _sendResponse(ERR_NOTREGISTERED(cli.getNickname()), fd);
      return ERR;
    }

    // Проверка на наличие получателей
    if (targets.empty()) {
        _sendResponse(ERR_NORECIPIENT(cli.getNickname()), fd);
        return ERR;
    }
    // Проверка на наличие текста сообщения
    if (message.empty()) {
        _sendResponse(ERR_NOTEXTTOSEND(cli.getNickname()), fd);
        return ERR;
    }
    // Проверка существования каналов и клиентов
    std::vector<std::string> recepients = CheckForChannelsAndClients(targets, fd); 

    // Отправка сообщения получателям
    for (size_t i = 0; i < 20 && i < recepients.size() ; ++i) {
        std::string response = CMD_PRIVMSG(cli.getHostname(), recepients[i], message);
        if (recepients[i][0] == '#') {
            std::string channelName = recepients[i].substr(1); // Удаляем символ '#'
            GetChannel(channelName)->sendToAll(response, fd);
        } else {
            _sendResponse(response, GetClientByNickname(recepients[i])->GetFd());
        }
    }

    if (recepients.size() > 20) {
        _sendResponse(ERR_TOOMANYTARGETS(cli.getNickname(), recepients[21]), fd);
    }

    return 0;
}
