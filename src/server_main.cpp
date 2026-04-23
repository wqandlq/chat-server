#include <iostream>

#include "ChatServer.h"

int main() {
    try {
        ChatServer server(8888);
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "server error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}