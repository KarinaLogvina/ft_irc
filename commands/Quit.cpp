#include "./includes/Server.hpp"
#include "Server.hpp"


void FindQuitCommand(std::string command, std::string tofind, std::string &str){
    size_t i = 0;
    for (size_t i = 0; i < command.size(); i++){
        if (command[i] != ' '){
            std::string tmp;
            for (; i < command.size() && command[i] != ' '; i++)
                tmp += command[i];
            if (tmp == tofind)
                break;
            else tmp.clear();
        }
    }
    if (i < command.size())
        str = command.substr(i);
    i = 0;
    for (size_t i = 0; i < str.size() && str[i] == ' '; i++);
    str = str.substr(i);
}

std::string SplitQuitCommand(std::string command){
    std::istringstream stm(command);
    std::string reason, str;
    stm >> str;
    FindQuitCommand(command, str, reason);
    if (reason.empty())
        return std::string("Quit");
    if (reason[0] != ':'){ //если сообщение не начинается с ':'
        for (size_t i = 0; i < reason.size(); i++){
            if (reason[i] == ' ') {
                reason.erase(reason.begin() + i, reason.end());
                break;
            }
        }
        reason.insert(reason.begin(), ':');
    }
    return reason;
}

void Server::handleClientQuit(int fd, const std::string& reason, Channel& channel) {
    std::string reply = ":" + GetClient(fd)->getNickname() + "!~" + GetClient(fd)->getUserName() + "@localhost QUIT " + reason + "\r\n";
    channel.sendToAll(reply);
    channel.removeClient(fd);
    if (channel.GetNumberOfClients() == 0) {
        // Удаляем канал, если в нем больше нет клиентов
        auto it = std::find(channels.begin(), channels.end(), channel);
        if (it != channels.end()) {
            channels.erase(it);
        }
    }
}

void Server::Quit(std::string command, int &fd) {
    std::string reason = SplitQuitCommand(command);

    for (auto& channel : channels) {
        if (channel.get_client(fd) || channel.get_admin(fd)) {
            handleClientQuit(fd, reason, channel);
        }
    }

    std::cout << "Client <" << fd << "> Disconnected" << std::endl;
    removeChannels(fd);
    removeClient(fd);
    removeFds(fd);
    close(fd);
}

