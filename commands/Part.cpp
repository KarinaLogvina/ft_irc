#include "../includes/Server.hpp"

std::vector<std::string> split(const std::string& str, char delimiter);

int Server::Part(Client& client) {
    std::string clientNick = client.getNickname();
    if (!client.getIsRegistered()) {
      _sendResponse(ERR_NOTREGISTERED(clientNick), client.GetFd());
      return ERR;
    }

    if (client_msg.params.size() < 1) {
        // Send error: need more parameters
        _sendResponse(ERR_NEEDMOREPARAMS(clientNick, "PART"), client.GetFd());
        return ERR;
    }

    std::string reason = client_msg.trailing.empty() ? "" : client_msg.trailing;  // If reason is provided, use it.

    // Split the channel names (comma-separated)
    std::vector<std::string> channels = split(client_msg.params[0], ',');

    for (size_t i = 0; i < channels.size(); i++) {
        if (channels[i][0] != '#') {
            _sendResponse(ERR_NOSUCHCHANNEL(client.getNickname(), channels[i][0]), client.GetFd());
            continue;
        }  
        std::string channelName = channels[i].substr(1);
        // Check if the channel exists
        Channel* channel = GetChannel(channelName);
        if (!channel) {
            // Channel does not exist
            _sendResponse(ERR_NOSUCHCHANNEL(client.getNickname(), channels[i]), client.GetFd());
            continue;
        }

        // Check if the client is part of the channel
        if (!channel->clientInChannel(clientNick)) {
            // Client is not on the channel
            _sendResponse(ERR_NOTONCHANNEL(clientNick, channelName), client.GetFd());
            continue;
        }

        // Client is part of the channel, remove them
        channel->removeClient(client.GetFd());
        channel->removeAdmin(client.GetFd());

        // Send PART message to all clients in the channel
		_sendResponse(CMD_PART(client.getHostname(), channelName, reason), client.GetFd());
        channel->sendToAll(CMD_PART(client.getHostname(), channelName, reason), client.GetFd());

        // Optionally, if no clients are left in the channel, delete it
        if (channel->isEmpty())
            removeChannel(channelName);
    }
	return 0;
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::stringstream ss(str);

    // Use getline to split based on the delimiter
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);  // Add the token to the vector if it's not empty
        }
    }

    return tokens;
}