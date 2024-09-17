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
  std::string clientNick = getClient(fd)->getNickname();
  if (s_cmd.size() < 3) {
    senderror(461, clientNick, fd, ": Not enough parameters\r\n");
    return ERR;
  }
  std::string channelname = s_cmd[2].substr(1);
  Channel * channel = GetChannel(channelname);
  if (s_cmd[2][0] != '#' || channel == NULL) {
    senderror(403, clientNick, fd, ": Channel not found\r\n");
    return ERR;
  }
  if(!(channel->get_client(fd)) && !(channel->get_admin(fd))) {
    senderror(403, clientNick, fd, ": You are not member of channel\r\n");
    return ERR;
  }
  if(channel->FindClientInChannel(s_cmd[1])) {
    senderror(443, clientNick, fd, ": This user is already member of this channel\r\n");
    return ERR;
  }
  Client *invitee = GetClientByNickname(s_cmd[1]);
  if(!invitee) {
    //must be error 401, fix asap and remake senderror function
    senderror(403, clientNick, fd, ":No such user\r\n");
    return ERR; 
  }
  if (channel->GetInvitOnly() && !channel->get_admin(fd)){
    sendChannelerror(482, channel->get_admin(fd)->getNickname(), s_cmd[2], fd, ": You don't have rights to invite users\r\n");
    return ERR; 
  }
  
  invitee->addChannelInvite(channelname);
  std::string responde1 = "341 " + clientNick + " " + invitee->getNickname() + " " + s_cmd[2]+"\r\n";
  _sendResponse(responde1, fd);
  std::string responde2 = ":" + getClient(fd)->getHostname() + " INVITE " + invitee->getNickname() + " :" + s_cmd[2]+"\r\n";
  _sendResponse(responde2, invitee->GetFd());
  return 0;
}