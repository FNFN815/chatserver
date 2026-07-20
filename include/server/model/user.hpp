#ifndef USER_H
#define USER_H

#include <string>
enum state_Type { online, offline };
//匹配User表的ORM类
class User {
public:
  User(unsigned int id = 0, std::string name = "", std::string password = "",
       state_Type state = offline) {
    this->name = name;
    this->state = state;
    this->id = id;
    this->password=password;
  }
  void setId(unsigned int id) { this->id = id; }
  void setName(std::string name) { this->name = name; }
  void setPwd(std::string pwd) { this->password = pwd; }
  void setState(state_Type state) { this->state = state; }

  unsigned int getId() { return this->id; }
  std::string getName() { return this->name; }
  std::string getPwd() { return this->password; }
  state_Type getState() { return this->state; }


  //转化枚举-》数据库字符串
  const char* stateToStr(state_Type st)
{
    switch(st)
    {
        case offline: return "offline";
        case online:  return "online";
        default:      return "offline";
    }
  }
  // 数据库字符串 → 枚举转换
  state_Type strToState(const char *str) {
    std::string s(str); 
    if(s == "offline")
        return offline;
    if(s=="online")
        return online;
    return offline;
}
private:
  unsigned int id;
  std::string name;
  std::string password;
  state_Type state; // DEFAULT
  
};
#endif