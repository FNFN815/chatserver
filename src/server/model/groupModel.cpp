#include "groupModel.hpp"
#include"db.h"
#include "group.hpp"
#include "groupUser.hpp"
#include <mysql/mysql.h>
// 创建群组
bool GroupModel::createGroup(Group &group) {
  // 组装字符串
  char sql[1024]{0};
  sprintf(sql, "insert into AllGroup(groupname,groupdesc) values('%s','%s')",
          group.getName().c_str(), group.getDesc().c_str());
  MySql mysql;
  if (mysql.connect()) {
    if (mysql.update(sql)) {
        group.setId(mysql_insert_id(mysql.getconnection()));
      return true;
    }
  }
  return false;
}
// 加入群组
void GroupModel::addGroup(unsigned int userid, unsigned int groupid,
                          string role) {
  // 组装字符串
  char sql[1024]{0};
  sprintf(sql,
          "insert into GroupUser(groupid,userid,grouprole) values(%u,%u,'%s')",
          groupid, userid, role.c_str());
  MySql mysql;
  if (mysql.connect()) {
    mysql.update(sql);
  }
}
// 查询用户所在群组信息

vector<Group> GroupModel::queryGroups(int userid) {
  /*1.先根据userid在groupuser表中查询到所属的群组id，grouprole
    2.根据群组id查询所属该群组的所有userid,并且和user表联合查询，查询用户的详细信息
*/
  // 组装字符串
  char sql[1024]{0};
  sprintf(sql,
          "select a.id ,a.groupname,a.groupdesc from AllGroup a inner join "
          "GroupUser b on a.id =b.groupid where b.userid =%u",
          userid);
  vector<Group> grouVec;
  MySql mysql;
  if (mysql.connect()) {
    MYSQL_RES *res = mysql.query(sql);
    if (res != nullptr) {
      // 查询userid 的所有群组信息
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(res)) != nullptr) {

        Group group;
        group.setId(atoi(row[0]));
        group.setName(row[1]);
        group.setDesc(row[2]);
        grouVec.push_back(group);
      }
      mysql_free_result(res);
    }
  }
  // 查询群组user
  for (Group &group : grouVec) {
    // 组装字符串
    char sql[1024]{0};
    sprintf(sql,
            "select a.id ,a.name,a.state, b.grouprole from User a inner join "
            "GroupUser b on b.userid =a.id where b.groupid =%u",
            group.getId());

    MySql mysql;
    if (mysql.connect()) {
      MYSQL_RES *res = mysql.query(sql);
      if (res != nullptr) {
        // 查询userid 的所有群组信息
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr) {

          GroupUser groupuser;
          groupuser.setId(atoi(row[0]));
          groupuser.setName(row[1]);
          groupuser.setState(groupuser.strToState(row[2]));
          groupuser.setRole(row[3]);
          group.getUsers().push_back(groupuser);
        }
        mysql_free_result(res);
      }
    }
  }
  return grouVec;
}
// 根据指定的群组id查询在其中的组员id
// 列表,除userid自己，主要用户群聊业务给其他成员发群消息
vector<unsigned int> GroupModel::queryGroupUsers(unsigned int userid,
                                                 unsigned int groupid) {
  // 组装字符串
  char sql[1024]{0};
  sprintf(sql,
          "select userid from GroupUser where Groupid = %u and userid !=%u",
          groupid, userid);
  vector<unsigned int> idVec;
  MySql mysql;
  if (mysql.connect()) {
    MYSQL_RES *res = mysql.query(sql);
    if (res != nullptr) {

      MYSQL_ROW row;
      while ((row = mysql_fetch_row(res)) != nullptr) {
        idVec.push_back(atoi(row[0]));
      }
      mysql_free_result(res);
    }
  }
  return idVec;
}