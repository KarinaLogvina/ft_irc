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
  if (!getClient(fd)->getIsRegistered()) {
    _sendResponse(ERR_NOTREGISTERED(getClient(fd)->getNickname()), fd);
    return ERR;
  }
  if (s_cmd.size() < 3) {
    senderror(461, clientNick, fd, ": Not enough parameters\r\n");
    return ERR;
  }
  std::string channelname = s_cmd[2].substr(1);
  Channel * channel = GetChannel(channelname);
  if (s_cmd[2][0] != '#' || channel == NULL) {
    _sendResponse(ERR_NOSUCHCHANNEL(clientNick, s_cmd[2]), fd);
    return ERR;
  }
  if(!(channel->get_client(fd)) && !(channel->get_admin(fd))) {
    _sendResponse(ERR_NOTONCHANNEL(clientNick, channelname), fd);
    return ERR;
  }
  if(channel->FindClientInChannel(s_cmd[1])) {
    _sendResponse(ERR_USERONCHANNEL(clientNick, s_cmd[1], channelname), fd);
    return ERR;
  }
  Client *invitee = GetClientByNickname(s_cmd[1]);
  if(!invitee) {
    _sendResponse(ERR_NOSUCHNICK(clientNick, s_cmd[1]), fd);
    return ERR; 
  }
  if (channel->GetInvitOnly() && !channel->get_admin(fd)){
    _sendResponse(ERR_CHANOPRIVSNEEDED(clientNick, channelname), fd);    return ERR; 
  }
  
  invitee->addChannelInvite(channelname);
  std::string responde1 = "341 " + clientNick + " " + invitee->getNickname() + " " + s_cmd[2]+"\r\n";
  _sendResponse(responde1, fd);
  std::string responde2 = ":" + getClient(fd)->getHostname() + " INVITE " + invitee->getNickname() + " :" + s_cmd[2]+"\r\n";
  _sendResponse(responde2, invitee->GetFd());
  return 0;
}