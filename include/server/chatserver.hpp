#ifndef CHARTSERVER_H
#define CHARTSERVER_H

#include <muduo/net/TcpConnection.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

//聊天服务器基类
class ChatServer
{
public:
  //初始化服务器对象
  ChatServer(EventLoop *loop, const InetAddress &listenAddr,
             const string &nameArg);
  //启动服务
    void start();

  private:
    // 上报连接相关信息的回调函数
    void onConnection(const TcpConnectionPtr &conn);
    //上报读写事件相关信息的回调函数
    void onMessage(const TcpConnectionPtr &conn, Buffer *buffer,
                   Timestamp timestamp);
  TcpServer _server; //组合的muduo库的TcpServer对象
  EventLoop* _loop;//事件循环指针
};


#endif // CHARTSERVER_H