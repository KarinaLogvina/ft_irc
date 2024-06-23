#ifndef FT_IRC_CHANNEL_HPP
#define FT_IRC_CHANNEL_HPP

#include "Client.hpp"

class Client;

class Channel {
  private:
    int topic;
    int key;
    int limit;
    std::string name;
    std::string password;
    std::string createdAt;
    std::string creationTime;
    std::string createdTime;
    std::string topicName;
    std::vector<Client> clients;
    std::vector<Client> admins;

  public:
    Channel();
    ~Channel();
    Channel(Channel const &src);
    Channel &operator=(Channel const &src);

    //---getters---
    int GetTopic();
    int GetKey();
    int GetLimit();
    int GetNumberOfClients();
    std::string GetChannelName();
    std::string GetPassword();
    std::string GetTimestamp();
    std::string clientChannel_list();
    Client *get_client(int fd);
    Client *get_admin(int fd);

    //---setters---
    void addClient(Client newClient);
    void addAdmin(Client newClient);
    void removeClient(int fd);
    void removeAdmin(int fd);
    bool changeClientToAdmin(std::string &nickname);
    bool removeClientAdminStatus(std::string &nickname);

    void sendToAll(std::string replay);

};
#endif //FT_IRC_CHANNEL_HPP
