#include "UserModel.hpp"
#include "db.h"
#include <cstdio>
#include <cstdlib>

#include <mysql/mysql.h>
using namespace std;
bool UserModel::insert(User &user) {
  // 组装字符串
  char sql[1024]{0};
  sprintf(sql, "insert into User(name,password) values('%s','%s')",
          user.getName().c_str(), user.getPwd().c_str());
  MySql mysql;
  if (mysql.connect()) {
    if (mysql.update(sql)) {
      // 获取插入成功的用户主键id
      user.setId(mysql_insert_id(mysql.getconnection()));  
      return true;
    }
  }
  return false;
}
// quray
User UserModel::query(unsigned int id) {
   // 组装字符串
  char sql[1024]{0};
  sprintf(sql, "select * from User where id =%u",id );
  MySql mysql;
  if (mysql.connect()) {
    MYSQL_RES *res = mysql.query(sql);
    if (res != nullptr) {
      
      MYSQL_ROW row = mysql_fetch_row(res);
      if (row != nullptr) {
        User user;
        user.setId(atoi(row[0]));
        user.setName(row[1]);
        user.setPwd(row[2]);
        // user.setState(atoi(row[3]));错误
        //  int stateVal = atoi(row[3]);
        //  user.setState((state_Type)stateVal);
        user.setState(user.strToState(row[3]));
        mysql_free_result(res);
        return user;
      }
    }
  }
  return User();
}
// 更新用户状态信息
bool UserModel::updateState(User user) {
  // 组装字符串
  char sql[1024]{0};
  sprintf(sql, "update User set state ='%s'  where id = %d",user.stateToStr(user.getState()),user.getId() );
  MySql mysql;
  if (mysql.connect()) {
    if (mysql.update(sql)) {
      return true;
    }
  }
  return false;
}
// 重置用户状态信息
void UserModel::resetState() {
   // 组装字符串
  char sql[1024]{"update User set state ='offline'  where state = 'online' "};

  MySql mysql;
  if (mysql.connect()) {
    mysql.update(sql);
  }
 
}