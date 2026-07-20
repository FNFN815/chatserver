#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H
#include <string>
#include <vector>
using namespace std;
// 提供用户离线消息表的相关操作
class OfflineMsgModel{
public:
  // 存储(插入)离线消息
  void insert(unsigned int userid, string msg);
  // 删除离线消息（已经发送给用户）
  void remove(unsigned int userid, unsigned int offlinemsgid);
   // 删除全部离线消息（已经发送给用户）
  void removeall(unsigned int userid);
  // 查询用户离线消息
  vector<string> query(unsigned int userid);
private:
  
};


#endif