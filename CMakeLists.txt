cmake_minimum_required(VERSION 3.10)
project(chat_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

add_executable(server
    src/server_main.cpp
    src/ChatServer.cpp
    src/ThreadPool.cpp
)

add_executable(client
    src/client.cpp
)


