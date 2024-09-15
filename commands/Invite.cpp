#include "../includes/Server.hpp"
/*
RPL_INVITING (341)
ERR_NEEDMOREPARAMS (461)
ERR_NOSUCHCHANNEL (403)
ERR_NOTONCHANNEL (442)
ERR_CHANOPRIVSNEEDED (482)
ERR_USERONCHANNEL (443)*/

int Server::Invite(std::string &cmd, int fd) {
  std::vector<std::string> s_cmd = split_command(cmd);
  if (s_cmd.size() < 3) {
    senderror(461, getClient(fd)->getNickname(), fd, ": Not enough parameters\r\n");
    return ERR;
  }
  std::string channelname = s_cmd[2].substr(1);
  if (s_cmd[2][0] != '#' || !GetChannel(channelname)) {
    senderror(403, getClient(fd)->getNickname(), fd, ": Channel no found\r\n");
    return ERR;
  }
  if(!(GetChannel(channelname)->get_client(fd)) && !(GetChannel(channelname)->get_admin(fd))) {
    senderror(403, getClient(fd)->getNickname(), fd, ": You are not member of channel\r\n");
    return ERR;
  }
  if(GetChannel(channelname)->FindClientInChannel(s_cmd[1])) {
    senderror(443, getClient(fd)->getNickname(), fd, ": This user is already member of this channel\r\n");
    return ERR;
  }
  Client *client = GetClientByNickname(s_cmd[1]);
  if(!client) {
    //must be error 401, fix asap and remake senderror function
    senderror(403, getClient(fd)->getNickname(), fd, ":No such user\r\n");
    return ERR; 
  }
  if (GetChannel(channelname)->GetInvitOnly() && !GetChannel(channelname)->get_admin(fd)){
    sendChannelerror(482, GetChannel(channelname)->get_client(fd)->getNickname(), s_cmd[1], fd, ": You are not have rights to invite user\r\n");
    return ERR; 
  }
  if(GetChannel(channelname)->GetLimit() && GetChannel(channelname)->GetNumberOfClients() >= GetChannel(channelname)->GetLimit()) {
    sendChannelerror(482, GetChannel(channelname)->get_client(fd)->getNickname(), s_cmd[1], fd, ": Channel is full of users.\r\n");
    return ERR;
  }
  
  client->addChannelInvite(channelname);
  std::string responde1 = "341 " + getClient(fd)->getNickname() + " " + client->getNickname() + " " + s_cmd[2]+"\r\n";
  _sendResponse(responde1, fd);
  std::string responde2 = " :" + client->getHostname() + " INVITE " + client->getNickname() + " " + s_cmd[2]+"\r\n";
  _sendResponse(responde2, client->GetFd());
  return 0;
}