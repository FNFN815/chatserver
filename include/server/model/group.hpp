#ifndef GROUP_H
#define GROUP_H
#include <vector>
#include <string>

#include"groupUser.hpp"
using namespace std;
class Group {
public:
  Group(unsigned int id = 0, string name = "", string desc = "") {
    this->id = id;
    this->name = name;
    this->desc=desc;
  }
  //set
  void setId(unsigned int id) { this->id = id; }
  void setName(string name) { this->name = name; }
  void setDesc(string desc) { this->desc = desc; }

  // get
  unsigned int getId() { return this->id; }
  string getName() { return this->name; }
  string getDesc() { return this->desc; }
  vector<GroupUser> & getUsers(){return this->users;}
private:
  unsigned int id;
  string name;
  string desc;
  vector<GroupUser> users;
};
#endif