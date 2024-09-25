#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#define ERR 1
#define UNKNOWN_CMD -1
#define CRLF "\r\n"

/* Normal responses */

#define RPL_WELCOME(client, user) "001 " + client + " :Welcome to the IRC Network, " + client + "!" + user + "@127.0.0.1\r\n"
#define RPL_YOURHOST(client) "002 " + client + " :Your host is 127.0.0.1, running version 1.0\r\n"
#define RPL_CREATED(client, datetime) "003 " + client + " :This server was created at: " + datetime + "\r\n"
#define RPL_MYINFO(client) "004 " + client + " 127.0.0.1 version 1.0, availiable channel modes: -itkl\r\n"
#define RPL_ISUPPORT(client) "005 " + client + " NETWORK=FT_IRC MAXCHANNELS=20 USERLEN=20 CHANMODES=,k,l,it CHANTYPES=# CHANLIMIT=#:20 : are supported by this server\r\n"
#define RPL_MOTD(client) "372 " + client + " :Dont't worry be happy!\r\n"
#define RPL_MOTDSTART(client) "375 " + client + " : - 127.0.0.1 Message of the day -\r\n"
#define RPL_ENDOFMOTD(client) "376 " + client + " :End of /MOTD command\r\n"
#define RPL_LUSERCLIENT(client, num_users) "251 " + client + " :There are " + num_users + " users on the server\r\n"
#define RPL_LUSERME(client, num_users) "255 " + client + " :I have " + num_users + " users and 1 server\r\n"

/* Error responses */

#define ERR_NEEDMOREPARAMS(client, command) "461 " + client + " " + command + " :Not enough parameters" + CRLF
#define ERR_ALREADYREGISTERED(client) "462 " + client + " :You have been already registered" + CRLF
#define ERR_PASSWDMISMATCH(client) "464 " + client + " :Incorrect password" + CRLF
#define ERR_NONICKNAMEGIVEN(client) "431 " + client + " :No nickname given" + CRLF
#define ERR_ERRONEUSNICKNAME(client, nick) "432 " + client + " " + nick + " :Erroneus nickname" + CRLF
#define ERR_NICKNAMEINUSE(client, nick) "433 " + client + " " + nick + " :Nickname is already in use" + CRLF
#define ERR_NICKCOLLISION(client, nick, user) "436 " + client + " " + nick + " :Nickname collision KILL from <user>@<host>" + CRLF
#define ERR_UNKNOWNCOMMAND(client, command) "421 " + client + " " + command + " :Unknown command" + CRLF

/* Normal commands */

#define CMD_NICK(hostname, newNick) (":" + hostname + " NICK " + newNick + CRLF)
#define CMD_PRIVMSG(source, target, text) (":" + source + " PRIVMSG " + target + " :" + text + CRLF)
#define CMD_KICK(hostname, channel, user, reason) (":" + hostname + " KICK " + channel + " " + user + " :" + reason + CRLF)
#define CMD_PART(hostname, channel, reason) (":" + hostname + " PART " + channel + " :" + reason + CRLF)

#endif