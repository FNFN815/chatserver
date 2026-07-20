#include "OfflineMessageModel.hpp"
#include "db.h"
#include <mysql/mysql.h>
#include <vector>
// 存储(插入)离线消息
void OfflineMsgModel::insert(unsigned int userid, string msg) {
  // 组装字符串
  char sql[1024]{0};
  sprintf(sql, "insert into OfflineMessage(userid,message) values(%u,'%s')", userid,
          msg.c_str());
  MySql mysql;
  if (mysql.connect()) {
    mysql.update(sql);
  }
}
  // 删除离线消息（已经发送给用户）
  void OfflineMsgModel::remove(unsigned int userid, unsigned int offlinemsgid) {
     // 组装字符串
  char sql[1024]{0};
  sprintf(sql, "delete from OfflineMessage where id =%u and userid = %u",offlinemsgid ,userid
         );
  MySql mysql;
  if (mysql.connect()) {
    mysql.update(sql);
  }
  }
  // 删除全部离线消息（已经发送给用户）
  void OfflineMsgModel::removeall(unsigned int userid) {
    // 组装字符串
    char sql[1024]{0};
    sprintf(sql, "delete from OfflineMessage where userid = %u", userid);
    MySql mysql;
    if (mysql.connect()) {
      mysql.update(sql);
    }
  }

  // 查询用户离线消息
  vector<string> OfflineMsgModel::query(unsigned int userid) {
    // 组装字符串
    char sql[1024]{0};
    sprintf(sql, "select message from OfflineMessage where userid =%u", userid);
    vector<string> vec;
    MySql mysql;
    if (mysql.connect()) {
      MYSQL_RES *res = mysql.query(sql);
      if (res != nullptr) {
        // 把用户的离线信息放入vec,返回
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr) {
          vec.push_back(row[0]);
        }
        mysql_free_result(res);
        return vec;
      }
    }
    return vec;
  }