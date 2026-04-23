#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <atomic>

std::atomic_bool running(true);

void recvMessage(int sockfd)
{
    while (true)
    {
        char buffer[1024] = {0};
        int ret = recv(sockfd, buffer, sizeof(buffer) - 1, 0);

        if (ret > 0)
        {
            std::cout << buffer << std::endl;
        }
        else if (ret == 0)
        {
            std::cout << "server closed connection\n";
            running = false;
            break;
        }
        else
        {
            std::cout << "recv failed\n";
            running = false;
            break;
        }
    }
}

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        std::cerr << "socket create failed\n";
        return 1;
    }

    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(sockfd, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == -1)
    {
        std::cerr << "connect failed\n";
        close(sockfd);
        return 1;
    }

    std::cout << "connected to server\n";

    std::thread recvThread(recvMessage, sockfd);

    while (true)
    {
        std::string input;
        if (!std::getline(std::cin, input))
        {
            break;
        }

        if (input == "quit")
        {
            running = false;
            shutdown(sockfd, SHUT_RDWR);
            break;
        }

        if ((send(sockfd, input.c_str(), input.size(), 0) == -1))
        {
            std::cerr << "send failed\n";
            running = false;
            break;
        }
    }

    if (recvThread.joinable())
    {
        recvThread.join();
    }

    close(sockfd);

    return 0;
}