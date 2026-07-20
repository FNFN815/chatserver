#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "user.hpp"
//群组角色继承user，多一个role成员以及相关方法
class GroupUser : public User {
public:
  // set
  void setRole(std::string role) { this->role = role; }

  // get
  std::string getRole(){return this->role;}
private:
  std::string role;
};
#endif