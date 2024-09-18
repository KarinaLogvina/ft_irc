#define CRLF "\r\n"

// Error Messages
#define ERR_NOTENOUGHTPARAMS(nick) (": 461 " + (nick) + " :Not enough params" + CRLF)
#define ERR_CHANNELNOTFOUND(channelname, nick) (": 403 " + (nick) + " " + (channelname) + " :Channel not found" + CRLF)
#define ERR_USERNOTREGISTERED(nick) (": 451 " + (nick) + " :Sorry. We didn't find your account." + CRLF)
#define ERR_INVALIDMODEPARM(channelname, mode) (": 696 #" + (channelname) + " Invalid mode parameter. " + (mode) + CRLF)
#define ERR_KEYSET(channelname) (": 467 #" + (channelname) + " Channel key already set. " + CRLF)
#define ERR_NOSUCHUSER(channelname, name) (": 401 #" + (channelname) + " " + (name) + " :No such user/channel" + CRLF)
#define ERR_NEEDMODEPARM(channelname, mode) (": 699 #" + (channelname) + " Need mode parameter. " + (mode) + CRLF)
#define ERR_NOTOPERATOR(channelname) (": 481 #" + (channelname) + " You are not an operator of the channel." + CRLF)
#define ERR_UNKNOWNMODE(username, channelname, mode) (": 472 " + (username) + " Unknown mode " + std::string(1, (mode)) + " for channel " + (channelname) + CRLF)
#define ERR_INVITEONLYCHAN(nickname, channelname) ("473 " + nickname + " #" + channelname + " :Cannot join channel (+i)" + CRLF)
#define ERR_CHANOPRIVSNEEDED(nickname, channelname) ("482 " + nickname + " #" + channelname + " :You're not channel operator" + CRLF)

// Responses
#define RPL_JOINMSG(nick, hostname, channelname) (":" + nick + "@" + hostname + " JOIN " + channelname + CRLF)
#define RPL_NAMREPLY(nickname, channelname, names) ("353 " + nickname + " = " + channelname + " :" + names + CRLF)
#define RPL_ENDOFNAMES(nickname, channelname) ("366 " + nickname + " " + channelname + " :END of /NAMES list" + CRLF)
#define RPL_TOPICIS(nickname, channelname, topic) ("332 " + (nickname) + " #" + (channelname) + " :" + (topic) + CRLF)
#define RPL_CHANNELMODES(nick, channelname, modes) ("324 " + (nick) + " #" + (channelname) + " :" + (modes) + CRLF)
#define RPL_CREATIONTIME(nick, channelname, timestamp) ("329 " + (nick) + " #" + (channelname) + " :" + (timestamp) + CRLF)
#define RPL_CHANGEMODE(hostname, channelname, mode, args) (":" + hostname + " MODE #" + (channelname) + " " + (mode) + " :" + (args) + CRLF)
