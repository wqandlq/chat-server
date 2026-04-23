#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <mutex>
#include <string>
#include <unordered_set>

#include "ThreadPool.h"

class ChatServer {
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
    void broadcastMessage(int senderFd, const std::string& msg);

   private:
    int port_;
    int listenfd_;
    int epfd_;
    std::unordered_set<int> clients_;
    ThreadPool threadPool_;
    std::mutex clientMtx_;
};

#endif