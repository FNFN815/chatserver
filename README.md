# chatserver

基于 muduo 网络库实现的分布式聊天服务器与客户端，适用于 nginx TCP 负载均衡场景。项目通过 Redis 发布/订阅机制实现跨节点消息转发，支持登录、注册、好友、群组和离线消息等能力。

## 项目简介

这是一个使用 C++ 编写的即时通讯服务端与客户端示例，核心目标是演示以下能力：

- 基于 muduo 的高性能 TCP 网络服务
- 支持多节点部署，并通过 nginx TCP 负载均衡接入
- 使用 Redis 的发布/订阅模式完成节点间消息传递
- 使用 MySQL 持久化用户、好友和群组相关数据
- 支持一对一聊天、群聊、好友添加、群组创建与加入、离线消息

## 技术栈

- C++11+
- muduo 网络库
- MySQL
- Redis
- hiredis
- CMake

## 项目结构

```text
.
├── bin/                  # 编译输出目录
├── build/                # CMake 构建目录
├── include/              # 头文件
│   └── server/
│       ├── db/
│       ├── model/
│       └── redis/
├── src/
│   ├── client/           # 客户端实现
│   └── server/           # 服务端实现
│       ├── db/
│       ├── model/
│       └── redis/
└── test/                 # 示例测试代码
```

## 功能特性

- 用户注册与登录
- 用户状态管理（在线/离线）
- 一对一私聊
- 添加好友
- 创建群组
- 加入群组
- 群组聊天
- 离线消息缓存
- Redis 订阅通道消息分发

## 构建要求

请确保本机已安装：

- CMake
- g++ / clang++
- MySQL
- Redis
- muduo 相关依赖

## 快速开始

### 1. 克隆项目

```bash
git clone https://github.com/FNFN815/chatserver.git
cd chatserver
```

### 2. 编译

```bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

编译完成后，二进制文件会生成到项目根目录下的 bin 目录：

- 服务端：bin/chatserver
- 客户端：bin/ChatClient

### 3. 配置数据库

当前代码默认使用 MySQL 数据库名 chat，用户名 root，密码 111111，地址 127.0.0.1。若你的本地环境不同，请先修改 [src/server/db/db.cpp](src/server/db/db.cpp) 中的连接配置。

建议先创建数据库：

```sql
CREATE DATABASE IF NOT EXISTS chat CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE chat;
```

然后根据业务需要创建相应表。项目中的数据模型主要包括用户、好友、群组和离线消息相关信息。

### 4. 启动 Redis

确认 Redis 服务已启动：

```bash
redis-server
```

### 5. 启动服务端

```bash
./bin/chatserver 127.0.0.1 6000
```

如需多节点部署，可在不同端口启动多个服务端实例，例如：

```bash
./bin/chatserver 127.0.0.1 6000
./bin/chatserver 127.0.0.1 6001
```

### 6. 启动客户端

```bash
./bin/ChatClient 127.0.0.1 6000
```

客户端启动后，支持以下交互命令：

- help：查看帮助
- chat:friendid:message：发送一对一消息
- addfriend:friendid：添加好友
- creategroup:groupname:groupdesc：创建群组
- addgroup:groupid：加入群组
- groupchat:groupid:message：发送群消息
- loginout：注销登录

## nginx TCP 负载均衡说明

该项目的设计目标之一，是让多个 chatserver 节点通过 nginx 的 TCP 代理对外提供统一入口。下面是一个简单的 nginx stream 配置示例：

```nginx
stream {
    upstream chat_backend {
        server 127.0.0.1:6000;
        server 127.0.0.1:6001;
    }

    server {
        listen 7000;
        proxy_pass chat_backend;
    }
}
```

客户端即可连接 nginx 的 7000 端口，而后端的多个 chatserver 节点会共同处理请求。

## 消息路由机制

项目通过 Redis 的发布/订阅机制完成跨节点消息投递：

- 用户登录成功后，会订阅自己的 Redis 通道
- 当目标用户不在线或当前节点无法直接命中时，消息会通过 Redis 发布到目标用户订阅的通道
- 目标节点收到消息后再推送给对应客户端

这样做的好处是：即使服务端节点之间没有直接连接，也可以在集群环境下将消息正确分发到目标用户。

## 说明

这个项目适合作为学习 muduo、Redis 发布/订阅、TCP 集群通信与 C++ 网络编程的入门示例。若你希望继续扩展，建议进一步加入：

- 更完整的数据库表结构和初始化脚本
- 更细粒度的消息协议版本管理
- 心跳检测与重连机制
- 更完善的异常处理和日志系统
