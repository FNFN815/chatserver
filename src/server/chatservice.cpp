#include "chatservice.hpp"
#include "group.hpp"
#include "groupUser.hpp"
#include "public.hpp"
#include "redis.hpp"
#include "user.hpp"
#include <muduo/base/Logging.h>
#include <mutex>
#include <vector>

using namespace muduo;
using namespace std;
// 获取单例对象的接口函数
ChatService *ChatService::instance() {
  static ChatService service;
  return &service;
}
// 注册消息以及对应的handler回调操作
ChatService::ChatService() {
  _msgHandlerMap.insert(
      {LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
  _msgHandlerMap.insert(
      {REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
  _msgHandlerMap.insert(
      {ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
  _msgHandlerMap.insert(
      {ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
  _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup,
                                                     this, _1, _2, _3)});
  _msgHandlerMap.insert(
      {GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});
  _msgHandlerMap.insert(
      {ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
  _msgHandlerMap.insert(
      {LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
  if (_redis.connect()) {
    //设置上报消息的回调
    _redis.init_notify_handler(std::bind(&ChatService::handlerRedisSubscribeMessage,this,_1,_2));
  }
}

// 登录方法
void ChatService::login(const TcpConnectionPtr &conn, json &js,
                        Timestamp timestamp) {
  LOG_INFO << "DO LOGIN";
  int id = js["id"].get<int>();
  string pwd = js["password"];
  LOG_INFO << "CURRUSERID: " << id << " PASSWORD: " << pwd;
  User user;
  user = _usermodel.query(id);
  LOG_INFO<<"query user-> id: "<<user.getId()<<" pwd: "<<user.getPwd()<<" state: "<<user.stateToStr(user.getState());
  if (user.getId() == id && user.getPwd() == pwd) {
    // 登录成功

    if (user.getState() == online) {
      // 已经登录，不允许重复登录
      json response;
      response["msgid"] = LOGIN_MSG_ACK;
      response["errno"] = 2;
      response["errmsg"] = "该用户已经登录";
      conn->send(response.dump());
      LOG_INFO<<"不允许重复登录: "<<response.dump();
    } else {
      {
        lock_guard<mutex> lock(_connMutex);
        // 登陆成功，记录连接信息
        _userConnMap.insert({id, conn});
      }

      // 用户（id）登录成功，向redis订阅channel：id
      _redis.subscribe(id);
      LOG_INFO<<"_redis.subscrible(id);";
      // 登录成功，更新用户状态信息
      user.setState(online);
      _usermodel.updateState(user);

      json response;
      response["msgid"] = LOGIN_MSG_ACK;
      response["errno"] = 0;
      response["id"] = user.getId();
      response["name"] = user.getName();
      // 检查该用户是否有离线信息
      vector<string> vec = _offlineMsgModel.query(id);
      if (!vec.empty()) {
        response["offlinemsg"] = vec;
        // 读取完离线信息后，删除离线信息

        _offlineMsgModel.removeall(id);
      }
      // 查询好友信息并返回
      vector<User> vec_friend = _friendModel.query(id);
      if (!vec_friend.empty()) {
        vector<string> vec_s_friend;
        for (auto it : vec_friend) {
          json js;
          js["id"] = it.getId();
          js["name"] = it.getName();
          js["state"] = it.stateToStr(it.getState());
          vec_s_friend.push_back(js.dump());
        }
        response["friends"] = vec_s_friend;
      }
      // 查询用户的群组信息
      vector<Group> groupUserVec = _groupModel.queryGroups(id);
      if (!groupUserVec.empty()) {
        vector<string> groupVec;
        for (Group &group : groupUserVec) {
          json groupjs;
          groupjs["id"] = group.getId();
          groupjs["groupname"] = group.getName();
          groupjs["groupdesc"] = group.getDesc();
          vector<string> userVec;
          for (GroupUser &user : group.getUsers()) {
            json userjs;
            userjs["id"] = user.getId();
            userjs["name"] = user.getName();
            userjs["state"] = user.stateToStr(user.getState());
            userjs["role"] = user.getRole();
            userVec.push_back(userjs.dump());
          }
          groupjs["users"] = userVec;
          groupVec.push_back(groupjs.dump());
        }
        response["groups"] = groupVec;
      }
      LOG_INFO<<"登录成功: "<<response.dump();
      conn->send(response.dump());
    }

  } else {
    // 用户存在 ，登录失败
    json response;
    response["msgid"] = LOGIN_MSG_ACK;
    response["errno"] = 1;
    response["errmsg"] = "用户名或密码错误";
    // response["id"] = user.getId();
    // response["name"]=user.getName();
    LOG_INFO<<"登录失败: "<<response.dump();
    conn->send(response.dump());
  }
}
// 注册方法
void ChatService::reg(const TcpConnectionPtr &conn, json &js,
                      Timestamp timestamp) {
  LOG_INFO << "DO REG";
  string name = js["name"];
  string passwordd = js["password"];
  User user;
  user.setName(name);
  user.setPwd(passwordd);
  bool state_REG = _usermodel.insert(user);
  if (state_REG) {
    // 注册成功
    json response;
    response["msgid"] = REG_MSG;
    response["errno"] = 0;
    response["id"] = user.getId();
    conn->send(response.dump());
  } else {
    // 注册失败
    json response;
    response["msgid"] = REG_MSG;
    response["errno"] = 1;
    conn->send(response.dump());
  }
}

// 获取消息的处理器
MsgHandler ChatService::getHandler(int msgid) {
  auto it = _msgHandlerMap.find(msgid);
  if (it == _msgHandlerMap.end()) {
    return [=](const TcpConnectionPtr &conn, json &js, Timestamp timestamp) {
      LOG_ERROR << "MSGID: " << msgid << " can not find handler!";
    };
  }
  return _msgHandlerMap[msgid];
}
  // 登录注销业务
  void ChatService::loginout(const TcpConnectionPtr &conn, json &js,
                             Timestamp timestamp) {
    int userid = js["id"].get<int>();
    {
      lock_guard<mutex> lock(_connMutex);
      auto it = _userConnMap.find(userid);
      if (it != _userConnMap.end()) {
        _userConnMap.erase(it);
      }
    }
    // 用户下线，在redis中注销订阅通道
    _redis.unsubcribe(userid);
    //更新状态
    User user(userid,"","",offline);
    _usermodel.updateState(user);
  }
// 客户端异常退出
void ChatService::clentCloseException(const TcpConnectionPtr &conn) {
  User user;
  {
    lock_guard<mutex> lock(_connMutex);
    
    for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it) {
      if (it->second == conn) {
        // 从Map表中删除连接信息
        user.setId(it->first);
        _userConnMap.erase(it);
        break;
      }
    }
  }
    // 用户下线，在redis中注销订阅通道
  _redis.unsubcribe(user.getId());

  // 更新用户的状态信息
  if (user.getId() != 0) { //如果循环中未找到——>userid=0(默认)->不向数据库查询，修改
    user.setState(offline);
    _usermodel.updateState(user);
  }
}
 // 一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js,
                          Timestamp timestamp) {
  int toid = js["toid"].get<int>();

  {
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(toid);
    if (it != _userConnMap.end()) {
      // 在线,转发消息

      it->second->send(js.dump());
      return;
    }
  }
  // 查询toid是否在线
  User user = _usermodel.query(toid);
  if (user.getState() == online) {
    _redis.publish(toid, js.dump());
    return;
  }
  // 不在线,存储离线消息
  _offlineMsgModel.insert(toid, js.dump());
}
  // 服务器异常，业务重置方法
void ChatService::reset() {
  // 把state:online 的用户的 state 设置成 offline
  _usermodel.resetState();
}
// 添加好友业务 MSGID ID FRIENDID
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js,
                            Timestamp timestamp) {
  int userid = js["id"].get<int>();
  int friendid = js["friendid"].get<int>();
  // 存储好友信息
  _friendModel.insert(userid, friendid);
}
// 群组
  // 1.创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp timestamp) {
  unsigned int userid = js["id"].get<int>();
  string groupname = js["groupname"];
  string groupdesc = js["groupdesc"];

  // 存储新创建的群组信息
  Group group(0, groupname, groupdesc);
  if (_groupModel.createGroup(group))
    // 存储群组创建人信息
    _groupModel.addGroup(userid, group.getId(), "creator");
  }
  //2.加入群组业务
  void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp timestamp) {
    unsigned int userid = js["id"].get<int>();
    unsigned int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
  }
  // 3.群组聊天
  void ChatService::groupChat(const TcpConnectionPtr &conn, json &js,
                              Timestamp timestamp) {
    unsigned int userid = js["id"];
    unsigned int groupid = js["groupid"];
    vector<unsigned int> useridVec =
        _groupModel.queryGroupUsers(userid, groupid);
    lock_guard<mutex> lock(_connMutex);
    for (int id : useridVec) {

      auto it = _userConnMap.find(id);
      // 转发群消息
      if (it != _userConnMap.end()) {
        it->second->send(js.dump());
      } else {
        // 查询toid是否在线
        User user = _usermodel.query(userid);
        if (user.getState() == online) {
          _redis.publish(id, js.dump());
         
        } else {
          // 存储离线消息
          _offlineMsgModel.insert(id, js.dump());
        }
      }
    }
  }
  // redis
  //从消息队列获得订阅的消息
  void ChatService::handlerRedisSubscribeMessage(int userid, string msg) {

   
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end()) {
      it->second->send(msg);
      return;
    }
    // 存储离线消息
    _offlineMsgModel.insert(userid, msg);

  }