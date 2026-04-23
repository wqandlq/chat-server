# Chat Server

一个基于 C++ 实现的多人聊天室服务器，使用 epoll 进行 I/O 多路复用，使用非阻塞 socket 与 ET 边沿触发管理连接和消息读写，并接入自实现线程池异步处理广播任务。

当前版本支持：
- 多客户端同时在线
- 消息广播
- 客户端连接/断开处理
- 基于线程池的异步广播任务处理

## 技术栈

- C++17
- Linux Socket API
- epoll
- 非阻塞 I/O
- ET 边沿触发
- 自实现线程池
- CMake
- std::mutex / std::lock_guard


## 项目结构

chat_server/
├── CMakeLists.txt
├── include/
│   ├── ChatServer.h
│   └── ThreadPool.h
├── src/
│   ├── ChatServer.cpp
│   ├── ThreadPool.cpp
│   ├── server_main.cpp
│   └── client.cpp

## 核心设计

### 1. 服务端主线程
主线程负责：
- epoll_wait 等待事件
- 处理新连接 accept
- 读取客户端消息 recv
- 将广播任务提交给线程池

### 2. 线程池
线程池负责异步执行广播任务，避免主线程在遍历连接和发送消息时阻塞过久。

### 3. 在线客户端管理
服务端使用 unordered_set<int> 保存在线客户端 fd，并通过 mutex 保护多线程访问安全。

### 4. 非阻塞 + ET
服务端将 listenfd 和 connfd 设置为非阻塞，并在 ET 模式下循环 accept/recv，直到返回 EAGAIN 或 EWOULDBLOCK。

## 编译运行

### 编译
mkdir build
cd build
cmake ..
make

### 运行服务端
./server

### 运行客户端
./client


## 当前已知问题

- 当前版本直接按 recv 返回的数据块广播，尚未实现应用层消息边界协议
- 在 TCP 字节流场景下，存在半包/粘包问题
- 对 send 返回值和发送不完整场景尚未做进一步处理
- 广播任务中仍可能遇到连接生命周期变化带来的边界问题，后续可通过更严格的连接管理策略优化

## 后续优化方向

- 增加基于分隔符或长度头的消息协议，解决半包/粘包问题
- 完善发送缓冲区与 send 不完整处理
- 增加日志模块
- 支持用户名、上线/下线系统消息
- 引入更完整的 Reactor/One Loop Per Thread 结构