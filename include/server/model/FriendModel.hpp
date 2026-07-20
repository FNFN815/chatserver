#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
#include <vector>
#include "user.hpp"
using namespace  std;
class FriendModel {
public:
  // 添加好友关系
  void insert(unsigned int userid, unsigned int friendid);
  // 返回用户好友列表
  vector<User> query(unsigned int userid);
};
#endif