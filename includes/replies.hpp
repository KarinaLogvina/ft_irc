#define once

#define CRLF "\r\n";

#define ERR_NOTENOUGHTPARAMS(nick)(": 461 #"+ nick + " : Not enought params" + CRLF);
#define ERR_CHANNELNOTFOUND(channelname, nick)(" : 403" + nick + " " + channelname + " Channel not found" + CRLF);
#define ERR_USERNOTREGISTERED(nick) ("451 " + nick + " :Sorry. We didn't find your account.\r\n");