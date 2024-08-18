#define once

#define CRLF "\r\n" 

#define ERR_NOTENOUGHTPARAMS(nick) (": 461 #" + nick + " : Not enough params" + CRLF)
#define ERR_CHANNELNOTFOUND(channelname, nick) (": 403 " + nick + " " + channelname + " :Channel not found" + CRLF)
#define ERR_USERNOTREGISTERED(nick) (": 451 " + nick + " :Sorry. We didn't find your account." + CRLF)

#define RPL_JOINMSG(hostname, ipaddress, channelname) (":" + hostname + "@" + ipaddress + " JOIN #" + channelname + CRLF)
#define RPL_NAMREPLY(nickname, channelname, clientslist) (": 353 " + nickname + " @ #" + channelname + " :" + clientslist + CRLF)
#define RPL_ENDOFNAMES(nickname, channelname) (": 366 " + nickname + " #" + channelname + " :END of /NAMES list" + CRLF)
#define RPL_TOPICIS(nickname, channelname, topic) (": 332 " + nickname + " #" +channelname + " :" + topic + CRLF)

