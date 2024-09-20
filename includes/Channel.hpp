#ifndef FT_IRC_CHANNEL_HPP
#define FT_IRC_CHANNEL_HPP

#include "Client.hpp"
#include <sstream>
#include <ctime> 


class Client;

class Channel {
  private:
    int topic;
    int key;
    int limit;
    int is_invite_only;
    std::string name;
    std::string password;
    std::string createdAt;
    std::string topicTimeStamp;
    std::string createdTime;
    std::string topicName;
    std::string changedBy;
    bool topic_restriction;
    std::vector<Client> clients;
    std::vector<Client> admins;
    std::vector<std::pair<char, bool> > modes;

  public:
    Channel();
    ~Channel();
    Channel(Channel const &src);
    Channel &operator=(Channel const &src);
    bool operator==(const Channel &other);


    //---getters---
    int GetTopic();
    int GetKey();
    int GetLimit();
    int GetNumberOfClients();
    int GetInvitOnly();
    std::string GetChannelName() const;
    std::string GetTopicName(){return this->topicName;}
    bool GetTopicRestriction() const{return this->topic_restriction;}
    std::string getTopicTimeStamp() const;
    std::string GetPassword();
    std::string GetTimestamp();
    std::string clientChannel_list();
    std::string getChangedBy() const;

    Client *get_client(int fd);
    Client *get_admin(int fd);
    Client *FindClientInChannel(std::string name);

    //---setters---
    void addClient(Client newClient);
    void addAdmin(Client newClient);
    void removeClient(int fd);
    void removeAdmin(int fd);
    void SetInvitOnly(int invit_only);
    void SetTopicTimeStamp(std::string time);
  	void SetTopic(int topic);
  	void SetKey(int key);
  	void SetLimit(int limit);
  	void SetTopicName(std::string topic_name);
  	void SetPassword(std::string password);
    void SetName(std::string &name);
    void setCreateiontime();
    bool changeClientToAdmin(std::string &nickname);
    bool changeAdminToClient(std::string &nick);
    std::string getModes() const;
    bool removeClientAdminStatus(std::string &nickname);

    void sendToAll(std::string replay, int except_fd = 0);
    void sendToAllExcept(std::string rpl1, int fd);
    bool getModeAtindex(size_t index);
    void setModeAtindex(size_t index, bool mode);
    void setTopicRestriction(bool value);

    bool clientInChannel(std::string &nick);
    std::string TopicTimeStamp();
    void setChangedBy(std::string nickname);
    bool isEmpty() const;
};


#endif //FT_IRC_CHANNEL_HPP
