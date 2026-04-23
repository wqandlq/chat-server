#include "ChatServer.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <cerrno>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

ChatServer::ChatServer(int port)
    : port_(port), listenfd_(-1), epfd_(-1)
{
}

ChatServer::~ChatServer()
{
    for (int fd : clients_)
    {
        close(fd);
    }

    if (listenfd_ != -1)
    {
        close(listenfd_);
    }

    if (epfd_ != -1)
    {
        close(epfd_);
    }
}

void ChatServer::start()
{
    initSocket();
    initEpoll();

    std::cout << "server is listening on port " << port_ << "...\n";

    epoll_event events[1024];

    while (true)
    {
        int nfds = epoll_wait(epfd_, events, 1024, -1);
        if (nfds == -1)
        {
            throw std::runtime_error("epoll_wait failed");
        }

        for (int i = 0; i < nfds; ++i)
        {
            int fd = events[i].data.fd;

            if (fd == listenfd_)
            {
                handleNewConnection();
            }
            else
            {
                handleClientMessage(fd);
            }
        }
    }
}

void ChatServer::initSocket()
{
    listenfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd_ == -1)
    {
        throw std::runtime_error("socket create failed");
    }

    setNonBlocking(listenfd_);

    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port_);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd_, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == -1)
    {
        throw std::runtime_error("bind failed");
    }

    if (listen(listenfd_, 128) == -1)
    {
        throw std::runtime_error("listen failed");
    }
}

void ChatServer::initEpoll()
{
    epfd_ = epoll_create1(0);
    if (epfd_ == -1)
    {
        throw std::runtime_error("epoll_create1 failed");
    }

    epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listenfd_;

    if (epoll_ctl(epfd_, EPOLL_CTL_ADD, listenfd_, &ev) == -1)
    {
        throw std::runtime_error("epoll_ctl add listenfd failed");
    }
}

void ChatServer::handleNewConnection()
{
    while (true)
    {
        sockaddr_in clientAddr;
        std::memset(&clientAddr, 0, sizeof(clientAddr));
        socklen_t clientLen = sizeof(clientAddr);

        int connfd = accept(listenfd_, reinterpret_cast<sockaddr *>(&clientAddr), &clientLen);
        if (connfd == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            std::cerr << "accept failed\n";
            break;
        }

        setNonBlocking(connfd);

        char clientIp[INET_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, sizeof(clientIp));
        std::cout << "new client connected, connfd = " << connfd
                  << ", ip = " << clientIp
                  << ", port = " << ntohs(clientAddr.sin_port)
                  << std::endl;

        epoll_event clientEv;
        std::memset(&clientEv, 0, sizeof(clientEv));
        clientEv.events = EPOLLIN | EPOLLET;
        clientEv.data.fd = connfd;

        if (epoll_ctl(epfd_, EPOLL_CTL_ADD, connfd, &clientEv) == -1)
        {
            std::cerr << "epoll_ctl add connfd failed\n";
            close(connfd);
            continue;
        }
        clients_.insert(connfd);
    }
}

void ChatServer::handleClientMessage(int fd)
{
    while (true)
    {
        char buffer[1024] = {0};
        int ret = recv(fd, buffer, sizeof(buffer) - 1, 0);

        if (ret > 0)
        {
            std::string msg = "client[" + std::to_string(fd) + "]: " + std::string(buffer);

            std::cout << msg << std::endl;
            
            for (int clientfd : clients_)
            {
                if (clientfd != fd)
                {
                    send(clientfd, msg.c_str(), msg.size(), 0);
                }
            }
        }
        else if (ret == 0)
        {
            std::cout << "client[" << fd << "] closed connection\n";
            removeClient(fd);
            break;
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }

            std::cerr << "recv failed, fd = " << fd << std::endl;
            removeClient(fd);
            break;
        }
    }
}

void ChatServer::removeClient(int fd)
{
    clients_.erase(fd);
    epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
}

void ChatServer::setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        throw std::runtime_error("fcntl F_GETFL failed");
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        throw std::runtime_error("fcntl F_SETFD failed");
    }
}