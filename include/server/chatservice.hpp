#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/base/Timestamp.h>
#include <muduo/net/Callbacks.h>
#include<muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>
#include <mutex>

#include"redis.hpp"
#include "UserModel.hpp"
#include "OfflineMessageModel.hpp"
#include "FriendModel.hpp"
#include"groupModel.hpp"
using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;
//处理消息的事件回调类型
using MsgHandler=std::function<void(const TcpConnectionPtr& conn,json &js,Timestamp timestamp)>;
class ChatService {
public:
  // 获取单例对象的接口函数
  static ChatService* instance();
  // 登录方法
  void login(const TcpConnectionPtr &conn, json &js, Timestamp timestamp);
  // 注册方法
  void reg(const TcpConnectionPtr &conn, json &js, Timestamp timestamp);
  // 一对一聊天业务
  void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp timestamp);
  // 添加好友业务
  void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp timestamp);
  // 获取消息的处理器
  MsgHandler getHandler(int msgid);
  // 登录注销业务
  void loginout(const TcpConnectionPtr &conn, json &js, Timestamp timestamp);
  // 处理客户端异常退出
  void clentCloseException(const TcpConnectionPtr &conn);
  // 服务器异常，业务重置方法
  void reset();

  // 群组
  // 1.创建群组业务
  void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp timestamp);
  //2.加入群组业务
  void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp timestamp);
  // 3.群组聊天
  void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp timestamp);

  // redis
  void handlerRedisSubscribeMessage(int ,string);
  //心跳检测与重连
  void heartbeat(const TcpConnectionPtr &conn, json &js, Timestamp timestamp);
  //检查客户端心跳超时
  void checkClientAlive();
private:
  // 存储消息id和其对应的处理方法
  unordered_map<int, MsgHandler> _msgHandlerMap;
  ChatService();
  // 数据操作类
  UserModel _usermodel;
  OfflineMsgModel _offlineMsgModel;
  FriendModel _friendModel;
  GroupModel _groupModel;
  //redis操作类
  Redis _redis;
  //心跳检测
  unordered_map<int, Timestamp> _userHeartbeatMap;


  // 存储在线用户的通信连接
  unordered_map<int, TcpConnectionPtr> _userConnMap;
  // 定义互斥锁，保证_userConnMap线程安全
  mutex _connMutex;
  
};
#endif