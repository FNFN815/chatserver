#ifndef GROUPMODEL_H
#define GROUPMODEL_H
#include "group.hpp"
#include <vector>
class GroupModel {
public:
  // 创建群组
    bool createGroup(Group &group);
  // 加入群组
    void addGroup(unsigned int userid,unsigned int groupid,string role);
  // 查询用户所在群组信息
    vector<Group> queryGroups(int userid);
    // 根据指定的群组id查询在其中的组员id
    // 列表,除userid自己，主要用户群聊业务给其他成员发群消息
    vector<unsigned int> queryGroupUsers(unsigned int userid,unsigned int groupid);
};



#endif