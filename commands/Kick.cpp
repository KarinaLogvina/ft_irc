#include "../includes/Server.hpp"

std::string FindAndSplitCmdK(std::string &cmd, std::vector<std::string> &tmp) {
    // Разделение команды на части
    std::stringstream ss(cmd);
    std::string str, reason;
    int count = 3;

    while (ss >> str && count--)
        tmp.push_back(str);

    if (tmp.size() != 3)
        return std::string("");

    // Поиск подстроки и извлечение причины
    size_t i = 0;
    for (; i < cmd.size(); i++) {
        if (cmd[i] != ' ') {
            std::string tmpStr;
            for (; i < cmd.size() && cmd[i] != ' '; i++)
                tmpStr += cmd[i];
            if (tmpStr == tmp[2]) break;
            else tmpStr.clear();
        }
    }

    if (i < cmd.size()) reason = cmd.substr(i);

    i = 0;
    for (; i < reason.size() && reason[i] == ' '; i++);
    reason = reason.substr(i);

    return reason;
}

std::string Server::SplitCmdKick(std::string cmd, std::vector<std::string> &temp, std::string &user, int fd) {
    std::string reason = FindAndSplitCmdK(cmd, temp);

    if (temp.size() < 3) // проверка, если клиент отправил имя канала и клиента для исключения
        return std::string("");

    temp.erase(temp.begin());
    std::string str = temp[0]; 
    std::string str1;
    user = temp[1]; 
    temp.clear();

    for (size_t i = 0; i < str.size(); i++) { // разделение первой строки по ',' для получения названий каналов
        if (str[i] == ',') {
            temp.push_back(str1); 
            str1.clear();
        } else {
            str1 += str[i];
        }
    }
    temp.push_back(str1);

    for (size_t i = 0; i < temp.size(); i++) // удаление пустых строк
        if (temp[i].empty())
            temp.erase(temp.begin() + i--);

    if (reason[0] == ':')
        reason.erase(reason.begin());
    else { // обрезка до первого пробела
        for (size_t i = 0; i < reason.size(); i++) {
            if (reason[i] == ' ') {
                reason = reason.substr(0, i);
                break;
            }
        }
    }

    for (size_t i = 0; i < temp.size(); i++) { // удаление '#' из названия канала и проверка, если канал действителен
        if (*(temp[i].begin()) == '#')
            temp[i].erase(temp[i].begin());
        else {
            sendChannelerror(403, getClient(fd)->getNickname(), temp[i], getClient(fd)->GetFd(), " :No such channel\r\n");
            temp.erase(temp.begin() + i--);
        }
    }

    return reason;
}


int Server::Kick(std::string cmd, int fd)
{
    std::vector<std::string> tmp;
    std::string user, reason;
    std::string clientNick = getClient(fd)->getNickname();
    Client & cli = *getClient(fd);

    // Используем объединённую функцию для разбора команды
    reason = FindAndSplitCmdK(cmd, tmp);

    if (!cli.getIsRegistered()) {
      _sendResponse(ERR_NOTREGISTERED(getClient(fd)->getNickname()), fd);
      return ERR;
    }

    // Проверяем, указаны ли все необходимые параметры
    if (tmp.size() < 3) {
        senderror(461, getClient(fd)->getNickname(), getClient(fd)->GetFd(), " :Not enough parameters\r\n");
        return ERR;
    }

    // Извлекаем имена каналов и пользователя
    tmp.erase(tmp.begin());
    std::string channelsStr = tmp[0];
    user = tmp[1];
    tmp.clear();

    // Разделяем имена каналов, удаляем пустые строки
    std::string channelName;
    for (size_t i = 0; i < channelsStr.size(); i++) {
        if (channelsStr[i] == ',') {
            if (!channelName.empty()) {
                tmp.push_back(channelName);
                channelName.clear();
            }
        } else {
            channelName += channelsStr[i];
        }
    }
    if (!channelName.empty()) {
        tmp.push_back(channelName);
    }

    // Проверяем валидность каждого канала
    for (size_t i = 0; i < tmp.size(); i++) {
        if (tmp[i].empty()) {
            tmp.erase(tmp.begin() + i--);
            continue;
        }

        if (tmp[i][0] != '#') {
            sendChannelerror(403, getClient(fd)->getNickname(), tmp[i], getClient(fd)->GetFd(), " :No such channel\r\n");
            tmp.erase(tmp.begin() + i--);
            continue;
        }

        tmp[i].erase(0, 1); // Удаляем символ '#'
    }

    // Обрабатываем каждый канал
    for (std::vector<std::string>::iterator channel = tmp.begin(); channel != tmp.end(); channel++) {
        Channel* ch = GetChannel(*channel);
        if (!ch) {
            sendChannelerror(403, getClient(fd)->getNickname(), "#" + *channel, getClient(fd)->GetFd(), " :No such channel\r\n");  //FIXME
            continue;
        }
        // Проверяем, является ли пользователь администратором или участником канала
        if (!ch->get_client(fd) && !ch->get_admin(fd)) {
            sendChannelerror(442, getClient(fd)->getNickname(), "#" + *channel, getClient(fd)->GetFd(), " :You're not on that channel\r\n"); //FIXME
            continue;
        }

        if (!ch->get_admin(fd)) {
            sendChannelerror(482, getClient(fd)->getNickname(), "#" + *channel, getClient(fd)->GetFd(), " :You're not channel operator\r\n"); //FIXME
            continue;
        }

        // Проверяем, находится ли пользователь в канале
        if (!ch->FindClientInChannel(user)) {
            _sendResponse(ERR_USERNOTINCHANNEL(clientNick, user, "#"+(*channel)), fd);
            continue;
        }

        // Формируем и отправляем сообщение об исключении
        
        ch->sendToAll(CMD_KICK(cli.getHostname(), "#"+(*channel), user, reason));

        // Удаляем пользователя из канала
        if (ch->get_admin(ch->FindClientInChannel(user)->GetFd())) {
            ch->removeAdmin(ch->FindClientInChannel(user)->GetFd());
        } else {
            ch->removeClient(ch->FindClientInChannel(user)->GetFd());
        }

        // Удаляем канал, если он пуст HE РАБОТАЕТ, НУЖНО ПЕРЕДЕЛАТЬ!!!!!
        // if (ch->GetNumberOfClients() == 0) {
        //     channels.erase(std::remove(channels.begin(), channels.end(), ch), channels.end());
        // }
    }
    return 0;
}
