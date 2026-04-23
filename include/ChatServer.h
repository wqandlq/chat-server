#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <unordered_set>

class ChatServer
{
public:
    ChatServer(int port = 8888);
    ~ChatServer();
    void start();

private:
    void initSocket();
    void initEpoll();
    void handleNewConnection();
    void handleClientMessage(int fd);
    void removeClient(int fd);

    void setNonBlocking(int fd);
private:
    int port_;
    int listenfd_;
    int epfd_;
    std::unordered_set<int> clients_;
};

#endif