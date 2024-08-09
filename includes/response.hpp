#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#define ERR 1
#define UNKNOWN_CMD -1

/* Normal responses */

#define RPL_WELCOME(client, user) "001 " + client + " :Welcome to the IRC Network, " + client + "!" + user + "@127.0.0.1\r\n"
#define RPL_YOURHOST(client) "002 " + client + " :Your host is 127.0.0.1, running version 1.0\r\n"
#define RPL_CREATED(client, datetime) "003 " + client + " :This server was created at: " + datetime + "\r\n"
#define RPL_MYINFO(client) "004 " + client + " 127.0.0.1 version 1.0, availiable channel modes: -itko\r\n"
#define RPL_ISUPPORT(client) "005 " + client + " USERLEN=18 CHANMODES=i,t,k,o,l : are supported by this server\r\n"
#define RPL_MOTD(client) "372 " + client + " :Dont't worry be happy!\r\n"
#define RPL_MOTDSTART(client) "375 " + client + " : - 127.0.0.1 Message of the day -\r\n"
#define RPL_ENDOFMOTD(client) "376 " + client + " :End of /MOTD command\r\n"
#define RPL_LUSERCLIENT(client, num_users) "251 " + client + " :There are " + num_users + " users on the server\r\n"
#define RPL_LUSERME(client, num_users) "255 " + client + " :I have " + num_users + " users and 1 server\r\n"

/* Error responses */

#define ERR_NOSUCHNICK(client, nick) "401 " + client + " " + nick + " :No such nick/channel" + "\r\n"
#define ERR_NEEDMOREPARAMS(client, command) "461 " + client + " " + command + " :Not enough parameters" + "\r\n"
#define ERR_ALREADYREGISTERED(client) "462 " + client + " :You have been already registered" + "\r\n"
#define ERR_PASSWDMISMATCH(client) "464 " + client + " :Incorrect password" + "\r\n"
#define ERR_NONICKNAMEGIVEN(client) "431 " + client + " :No nickname given" + "\r\n"
#define ERR_ERRONEUSNICKNAME(client, nick) "432 " + client + " " + nick + " :Erroneus nickname" + "\r\n"
#define ERR_NICKNAMEINUSE(client, nick) "433 " + client + " " + nick + " :Nickname is already in use" + "\r\n"
#define ERR_NICKCOLLISION(client, nick, user) "436 " + client + " " + nick + " :Nickname collision KILL from <user>@<host>" + "\r\n"

#endif