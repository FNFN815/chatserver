#ifndef PUBLIC_H
#define PUBLIC_H
// server client 的公共文件

enum EnMsgType {
  LOGIN_MSG = 1, // 登录消息
  LOGIN_MSG_ACK, // 登录响应消息
  LOGINOUT_MSG,   //登录注销
  REG_MSG,        // 注册消息
  REG_MSG_ACK,    // 注册响应消息
  ONE_CHAT_MSG,   // 聊天消息（one to one)
  ADD_FRIEND_MSG, // 添加好友

  CREATE_GROUP_MSG, // 创建群组消息
  ADD_GROUP_MSG,    // 添加到群组
  GROUP_CHAT_MSG,   //群聊天
  HEARTBEAT_MSG,    // 心跳消息
  HEARTBEAT_ACK_MSG // 心跳响应消息
};


#endif