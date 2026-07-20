#ifndef USERMODEL_H
#define USERMODEL_H
#include"user.hpp"
// User表的数据操作表
class UserModel {
public:
  // 增加user表
  bool insert(User &user);
  User query(unsigned int id);
  // 更新用户状态信息
  bool updateState(User user);
  // 重置用户状态信息
  void resetState();

};
#endif 