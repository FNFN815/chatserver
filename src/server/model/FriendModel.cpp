#include "FriendModel.hpp"
#include"db.h"
#include <cstdlib>
// 添加好友关系
void FriendModel::insert(unsigned int userid, unsigned int friendid) {
   // 组装字符串
  char sql[1024]{0};
  sprintf(sql, "insert into Friend(userid,friendid) values(%u,%u)", userid,
         friendid);
  MySql mysql;
  if (mysql.connect()) {
    mysql.update(sql);
  }
  }
  // 返回用户好友列表
vector<User> FriendModel::query(unsigned int userid) {
  // 组装字符串
    char sql[1024]{0};
    sprintf(sql, "select a.id ,a.name ,a.state from User a inner join Friend b on b.friendid = a.id where b.userid =%u", userid);
    vector<User> vec;
    MySql mysql;
    if (mysql.connect()) {
      MYSQL_RES *res = mysql.query(sql);
      if (res != nullptr) {
        // 把用户的离线信息放入vec,返回
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr) {
          User user;
          user.setId(atoi(row[0]));
          user.setName(row[1]);
          user.setState(user.strToState(row[2]));
          vec.push_back(user);
        }
        mysql_free_result(res);
        return vec;
      }
    }
    return vec;
  }