#include "chatserver.hpp"
#include <functional>
#include <nlohmann/json.hpp>
#include "chatservice.hpp"
using json = nlohmann::json;

using namespace std;
using namespace placeholders;
// 初始化服务器对象
ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr,
                       const string &nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop) {
  // 注册连接回调函数
  _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
  // 注册消息回调函数
  _server.setMessageCallback(
      std::bind(&ChatServer::onMessage, this, _1, _2, _3));
  // 设置合适的线程数量
  _server.setThreadNum(4);
  // 设置心跳检测定时器
  _loop->runEvery(10.0,std::bind(&ChatService::checkClientAlive, ChatService::instance()));
}
// 启动服务
void ChatServer::start() {
  _server.start(); // 调用muduo库中TcpServer对象的start方法
}
// 上报连接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr &conn) {
  // 客户端断开连接
  if (!conn->connected()) {

    ChatService::instance()->clentCloseException(conn);
    conn->shutdown();
  }
}
// 上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer,
                           Timestamp timestamp)
{
  string buf = buffer->retrieveAllAsString();
  //数据的反序列化
  json js = json::parse(buf);
  // 达到的目的：完全解耦网路模块代码和业务模块代码
  // 通过js[msgid]获取handler->conn js timestamp
  auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
  //回调消息绑定好的事件处理器，来执行相应的业务处理
  msgHandler(conn,js,timestamp);
}