
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <locale>
#include <nlohmann/json.hpp>

// c++
#include <chrono>
#include <ctime>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include<atomic>
using namespace std;
using json = nlohmann::json;

// linux
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include<semaphore.h>


#include "group.hpp"
#include "public.hpp"
#include "user.hpp"
#include "groupUser.hpp"
// 记录当前系统时间登录的用户信息
User g_currentUser;
// 记录当前用户好友的列表信息
vector<User> g_currentUserFriendList;
// 记录当前用户的群组列表信息
vector<Group> g_currentUserGroupList;
// 显示当前登录成功的用户信息
void showCurrentUserData();
// 用于读写线程的通信
sem_t rwsem;
// 记录登录状态
atomic_bool g_isLoginSuccess{false};
// 全局保存当前有效 socket fd，供各线程读取/更新
std::atomic<int> g_clientfd{-1};
// 处理登录响应的业务逻辑
void doLoginResponse(json &js);
// 处理注册响应的业务逻辑
void doRegResponse(json &js);
// 接收线程
void readTaskHandler(const char *ip, uint16_t port);
// 获取系统时间->聊天信息
string getCurrentTime();
// 客户端命令函数
void help(int =0, string="");
void chat(int, string);
void addfriend(int, string);
void creategroup(int, string);
void addgroup(int, string);
void groupchat(int, string);
void loginout(int, string);
//发送心跳
void sendHeartbeat(const char *ip, uint16_t port);
// 处理心跳响应（重连并更新 g_clientfd）
bool reconnectClient(const char *ip, uint16_t port);
// 控制聊天页面
bool isMMRunning=false;
// 主聊天页面程序
void mainMenu(int clientfd);
// 聊天客户端程序实现，main线程发送线程，子线程接收线程
int main(int argc, char **argv) {
  if (argc < 3) {
    cerr << "command invalid ! example: ./ChatClient 127.0.0.1 6000" << endl;
    exit(-1);
  }
  // 通过解析命令行参数传递ip , port
  char *ip = argv[1];
  uint16_t port = atoi(argv[2]);
  // 创建client端的socket
  int clientfd = socket(AF_INET, SOCK_STREAM, 0);
  if (clientfd == -1) {
    cerr << "create clientfd error " << endl;
    exit(-1);
  }
  // 填写client连接所需要的server信息 ip,port
  sockaddr_in server;
  memset(&server, 0, sizeof(sockaddr_in));
  server.sin_addr.s_addr = inet_addr(ip);
  server.sin_port = htons(port);
  server.sin_family = AF_INET;

  // client与server进行连接
  if (-1 == connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in))) {
    cerr << "connect error" << endl;
    close(clientfd);
    exit(-1);
  }
  // 初始化读写线程通信的信号量
  sem_init(&rwsem, 0, 0);
  // 记录当前有效 socket，连接成功后启动接收子线程（传入 ip/port）
  g_clientfd.store(clientfd);
  std::thread writeTask(readTaskHandler, ip, port);
  writeTask.detach();
  // main线程用于发送，接收数据
  for (;;) {
    // 显示首页面菜单，登录，注册，退出
    cout << "======================================================" << endl;
    cout << "1.LOGIN" << endl;
    cout << "2.REGISTER" << endl;
    cout << "3.quit" << endl;
    cout << "choice: " << endl;
    int choice = 0;
    cin >> choice;
    cin.get(); // 读掉缓冲区残留的回车
    switch (choice) {
      // 登录业务
    case 1: {
      int id = 0;
      char pwd[50]{0};
      cout << "userid: " << endl;
      cin >> id;
      cin.get(); // 读掉缓冲区残留的回车
      cout << "password: " << endl;
      cin.getline(pwd, 50);
      // json操作
      json loginjs;
      loginjs["msgid"] = LOGIN_MSG;
      loginjs["id"] = id;
      cout << "id:" << id << endl;
      loginjs["password"] = pwd;
      cout << "pwd: " << pwd << endl;
      string request = loginjs.dump();
      cout << "request:" << request << endl;

      //
      g_isLoginSuccess=false;
      int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
      if (len == -1) {
        cerr << "send login msg error" << request << endl;
      }

      //等待信号量 由子线程处理完登录的响应消息后，通知
      sem_wait(&rwsem);
      //登录成功
      if (g_isLoginSuccess) {
        // 启动心跳线程（使用全局 g_clientfd）
        std::thread heartbeatThread(sendHeartbeat, ip, port);
        heartbeatThread.detach();
        // 进入聊天菜单
        isMMRunning = true;
        mainMenu(clientfd);
      }
     
      
    }break;

      // reister
    case 2: {
      char name[50]{0};
      char password[50]{0};
      cout << "name: " << endl;
      cin.getline(name, 50);
      cout << "password: " << endl;
      cin.getline(password, 50);

      json reisterjs;
      reisterjs["msgid"] = REG_MSG;
      reisterjs["name"] = name;
      reisterjs["password"] = password;
      string request = reisterjs.dump();

      int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
      if (-1 == len) {
        cerr << "send resiter msg erroe :" << request << endl;
      }
      cout<<"sem_wait"<<endl;
      sem_wait(&rwsem);
      cout<<"sem"<<endl;
      
    }
    break;
      // 退出
    case 3: {
      close(clientfd);
      sem_destroy(&rwsem);
      exit(0);
    }
    default: {
      cerr << "invalid input" << endl;
    }
      break;
    }
  }
  return 0;
}

// 显示当前登录成功的用户信息
void showCurrentUserData() {
  cout << "=======================================================" << endl;
  cout << "current login user => id: " << g_currentUser.getId()
       << " name:" << g_currentUser.getName() << endl;
  cout << "------------------------friendList---------------------" << endl;
  if (!g_currentUserFriendList.empty()) {
    for (User &user : g_currentUserFriendList) {
      cout << user.getId() << " " << user.getName() << " "
           << user.stateToStr(user.getState()) << endl;
    }
  }
  cout << "------------------------groupList----------------------" << endl;
  if (!g_currentUserGroupList.empty()) {
    for (Group &group : g_currentUserGroupList) {
      cout << group.getId() << " " << group.getName() << " " << group.getDesc()
           << endl;
      cout<<"size of group: "<<group.getUsers().size()<<endl;;
      for (GroupUser &user : group.getUsers()) {
        cout << user.getId() << " " << user.getName() << " " << user.stateToStr(user.getState())
             << " " << user.getRole() << endl;
      }
    }
  }
  cout << "=======================================================" << endl;
}
// 接收线程（使用全局 g_clientfd）
void readTaskHandler(const char *ip, uint16_t port) {
  for (;;) {
    int fd = g_clientfd.load();
    if (fd <= 0) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }
    char buffer[4096];
    int len = recv(fd, buffer, sizeof(buffer), 0);
    if (len <= 0) {
      // 连接断开，关闭并清理当前 fd
      close(fd);
      g_clientfd.store(-1);
      // 尝试重连
      if (reconnectClient(ip, port)) {
        continue;
      }
      // 重连失败，退出进程
      exit(1);
    }
    // 使用接收的长度构建字符串并解析 JSON
    string msg(buffer, len);
    json js;
    try {
      js = json::parse(msg);
    } catch (const std::exception &e) {
      // 解析异常，跳过
      continue;
    }
    int msgType = js["msgid"].get<int>();
    if (ONE_CHAT_MSG == msgType) {
      cout << "Messages from orther " << js["time"].get<string>() << "  [ "
           << js["id"] << " ]  " << js["name"].get<string>()
           << "  said:" << js["msg"].get<string>() << endl;
      continue;
    }
    if (GROUP_CHAT_MSG == msgType) {
      cout << "Messages from group [" << js["groupid"] << "] "
           << js["time"].get<string>() << "  [ " << js["id"] << " ]  "
           << js["name"].get<string>() << "  said:" << js["msg"].get<string>()
           << endl;
      continue;
    }
    if (LOGIN_MSG_ACK == msgType) {
      doLoginResponse(js); // 处理登录响应的业务逻辑
      sem_post(&rwsem);
      continue;
    }
    if (REG_MSG_ACK == msgType) {
      doRegResponse(js);
      sem_post(&rwsem);
      continue;
    }
    if (HEARTBEAT_ACK_MSG == msgType) {
      // 心跳响应，保持连接活跃
      continue;
    }
  }
}
// 获取系统时间->聊天信息
string getCurrentTime() {
  auto curtime = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(curtime);
    // 跨平台安全本地时间转换
    std::tm tm_buf{};
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif
    char buf[64] = {0};
    // sprintf直接格式化到字符数组，不需要ostringstream / put_time
    std::sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
        tm_buf.tm_year + 1900,
        tm_buf.tm_mon + 1,
        tm_buf.tm_mday,
        tm_buf.tm_hour,
        tm_buf.tm_min,
        tm_buf.tm_sec
    );
    return std::string(buf);
 }
// 注册系统支持的客户端命令处理
unordered_map<string,  string> commandMap{
    {"help", "显示所有支持的命令，格式help"},
    {"chat", "一对一聊天，格式 chat:friendid:message"},
    {"addfriend", "添加好友，格式 addfriend:friendid"},
    {"creategroup", "创建群组，格式 creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组,格式 addgroup:groupid"},
    {"groupchat", "群聊，格式 groupchat:groupid:message"},
    {"loginout", "注销，格式 loginout"}

};
// 客户端命令函数
void help(int, string) {
  cout << "show all commandList" << endl;
  for (auto &comm : commandMap) {
    cout<<comm.first<<": "<<comm.second<<endl;
  }
  cout<<endl;
}
void chat(int clientfd, string str) {
  //friendid:msg
  int idx = str.find(":");
  if (-1 == idx) {
    cerr << "chat command invalid!" << endl;
    return;
  }
  int friendid = atoi(str.substr(0, idx).c_str());
  string msg = str.substr(idx + 1, str.size() - idx);
  json js;
  js["msgid"] = ONE_CHAT_MSG;
  js["id"] = g_currentUser.getId();
  js["name"] = g_currentUser.getName();
  js["toid"] = friendid;
  js["msg"] = msg;
  js["time"] = getCurrentTime();
  string buffer = js.dump();

  int fd = g_clientfd.load();
  if (fd <= 0) {
    cerr << "not connected, send failed" << endl;
    return;
  }
  int len = send(fd, buffer.c_str(), buffer.size(), 0);
  if (-1 == len) {
    cerr << "send chat msg error ->" << buffer << endl;
  }
}
void addfriend(int clientfd, string str) {
  int friendid = atoi(str.c_str());
  json js;
  js["msgid"] = ADD_FRIEND_MSG;
  js["id"]=g_currentUser.getId();
  js["friendid"] = friendid;
  string buffer = js.dump();
  int fd = g_clientfd.load();
  if (fd <= 0) {
    cerr << "not connected, send failed" << endl;
    return;
  }
  int len = send(fd, buffer.c_str(), buffer.size(), 0);
  if (-1 == len) {
    cerr << "send addfriend msg error" << buffer << endl;
  }
}
void creategroup(int clientfd, string str) {
  // groupname:groupdesc
  int userid = g_currentUser.getId();
  int idx = str.find(":");
  if (-1 == idx) {
    cerr << "creategroup command invalid!" << endl;
    return;
  }
  json js;
  js["msgid"]=CREATE_GROUP_MSG;
  js["id"] = userid;
  js["groupname"]=str.substr(0, idx);
  js["groupdesc"] = str.substr(idx + 1, str.size() - idx);
  string buffer{0};
  buffer=js.dump();
  int fd = g_clientfd.load();
  if (fd <= 0) {
    cerr << "not connected, send failed" << endl;
    return;
  }
  int len = send(fd, buffer.c_str(), buffer.size(), 0);
  if (-1 == len) {
     cerr << "send creategroup msg error" << buffer << endl;
  }
}
void addgroup(int clientfd, string str) {
  //addgroup:groupid
  int groupid = atoi(str.c_str());
  json js;
  js["msgid"] = ADD_GROUP_MSG;
  js["id"]=g_currentUser.getId();
  js["groupid"] = groupid;
  string buffer = js.dump();
  int fd = g_clientfd.load();
  if (fd <= 0) {
    cerr << "not connected, send failed" << endl;
    return;
  }
  int len = send(fd, buffer.c_str(), buffer.size(), 0);
  if (-1 == len) {
    cerr << "send addgroup msg error" << buffer << endl;
  }
}
void groupchat(int clientfd, string str) {
  // groupid:message
 
  int idx = str.find(":");
  if (-1 == idx) {
    cerr << "groupchat command invalid!" << endl;
    return;
  }
  int groupid = atoi(str.substr(0, idx).c_str());
  string msg = str.substr(idx + 1, str.size() - idx);
  json js;
  js["msgid"] = GROUP_CHAT_MSG;
  js["id"] = g_currentUser.getId();
  js["name"] = g_currentUser.getName();
  js["groupid"] = groupid;
  js["msg"] = msg;
  js["time"] = getCurrentTime();
  string buffer = js.dump();

  int fd = g_clientfd.load();
  if (fd <= 0) {
    cerr << "not connected, send failed" << endl;
    return;
  }
  int len = send(fd, buffer.c_str(), buffer.size(), 0);
  if (-1 == len) {
    cerr << "send groupchat msg error ->" << buffer << endl;
  }
}
void loginout(int clientfd, string str) {
   
  json js;
  js["msgid"] = LOGINOUT_MSG;
  js["id"]=g_currentUser.getId();
 
  string buffer = js.dump();
  int fd = g_clientfd.load();
  if (fd <= 0) {
    cerr << "not connected, send failed" << endl;
    return;
  }
  int len = send(fd, buffer.c_str(), buffer.size(), 0);
  if (-1 == len) {
    cerr << "send loginout msg error" << buffer << endl;
  } else {
    isMMRunning = false;
  }
}

// 注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)>> commandHandlerMap{
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}

};
// 主聊天页面程序
void mainMenu(int clientfd) {
  help(clientfd, "");
  char buffer[1014]{};
  while (isMMRunning) {
    cin.getline(buffer, 1024);
    string commandbuf(buffer);
    string command; // 存储命令
    int idx = commandbuf.find(":");
    if (-1 == idx) {
      command=buffer;
    } else {
      command=commandbuf.substr(0,idx);
    }
    auto it = commandHandlerMap.find(command);
    if (it == commandHandlerMap.end()) {
      cerr << "invalid input command" << endl;
      continue;
    }
    //调用相应的命令事件回调
    it->second(clientfd,commandbuf.substr(idx+1,commandbuf.size()-idx));
  }
}
// 处理登录响应的业务逻辑
void doLoginResponse(json &responsejs) {

  if (0 != responsejs["errno"].get<int>()) // 登录失败
  {
    cerr << responsejs["errmsg"] << endl;
    g_isLoginSuccess=false;
  } else // 登录成功
  {
    // 记录当前用户的信息
    g_currentUser.setId(responsejs["id"].get<int>());
    g_currentUser.setName(responsejs["name"]);
    // 记录当前用户的好友列表信息
    if (responsejs.contains("friends")) {
      // 初始化
      g_currentUserFriendList.clear();

      vector<string> vec = responsejs["friends"];
      for (string &str : vec) {
        json js;
        js = json::parse(str);
        User user;
        user.setId(js["id"].get<int>());
        user.setName(js["name"]);
        user.setState(user.strToState(js["state"].get<string>().c_str()));
        g_currentUserFriendList.push_back(user);
      }
    }
    // 记录当前用户的群组信息
    if (responsejs.contains("groups")) {
      // 初始化
      g_currentUserGroupList.clear();

      vector<string> vec = responsejs["groups"];
      for (string groupstr : vec) {
        json groupsjs;
        groupsjs = json::parse(groupstr);
        Group group;
        group.setId(groupsjs["id"].get<int>());
        group.setName(groupsjs["groupname"]);
        group.setDesc(groupsjs["groupdesc"]);

        vector<string> vec2 = groupsjs["users"];
        for (string &str : vec2) {
          GroupUser groupuser;
          json grpuserjs = json::parse(str);
          groupuser.setId(grpuserjs["id"].get<int>());
          groupuser.setName(grpuserjs["name"]);
          groupuser.setState(
              groupuser.strToState(grpuserjs["state"].get<string>().c_str()));
          groupuser.setRole(grpuserjs["role"]);
          group.getUsers().push_back(groupuser);
        }
        g_currentUserGroupList.push_back(group);
      }
    }
    showCurrentUserData();
    // 显示当前用户的离线消息，个人聊天信息，或群组消息
    if (responsejs.contains("offlinemsg")) {
      vector<string> vec = responsejs["offlinemsg"];
      for (string &str : vec) {
        json offjs = json::parse(str);
        if (ONE_CHAT_MSG == offjs["msgid"].get<int>()) {
          cout << "Messages from orther " << offjs["time"].get<string>()
               << "  [ " << offjs["id"] << " ]  " << offjs["name"].get<string>()
               << "  said:" << offjs["msg"].get<string>() << endl;
        } else {
          cout << "Messages from group [" << offjs["groupid"] << "] "
               << offjs["time"].get<string>() << "  [ " << offjs["id"] << " ]  "
               << offjs["name"].get<string>()
               << "  said:" << offjs["msg"].get<string>() << endl;
        }
      }
    }
    // // 登录成功启动接收线程，负责接收数据,只启动一次
    // static int readthreadnum = 0;
    // if (readthreadnum == 0) {
    //    std::thread writeTask(readTaskHandler, clientfd);
    //    writeTask.detach();
    //    readthreadnum++;
    // }

     g_isLoginSuccess=true;
  }
  
}
// 处理注册响应的业务逻辑
void doRegResponse(json &responsejs) {
  
  if (0 != responsejs["errno"].get<int>()) // 失败
  {
    cerr  << "name alreadly exist,resiter error" << endl;
  } else // 成功
  {
    cout  << " name resiter success, userid: " << responsejs["id"]
        << " do not forget it!" << endl;
  }
}
//发送心跳（使用全局 g_clientfd）
void sendHeartbeat(const char *ip, uint16_t port)
{
  while (g_isLoginSuccess) {
    int fd = g_clientfd.load();
    if (fd <= 0) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }
    json js;
    js["msgid"] = HEARTBEAT_MSG;
    js["id"] = g_currentUser.getId();
    string buffer = js.dump();
    ssize_t n = send(fd, buffer.data(), buffer.size(), 0);
    if (n <= 0) {
      cerr << "send heartbeat msg error" << endl;
      // 标记断开并尝试重连
      close(fd);
      g_clientfd.store(-1);
      reconnectClient(ip, port);
    }
    std::this_thread::sleep_for(std::chrono::seconds(10)); // 每10秒发送一次心跳
  }
}

// 处理心跳响应（重连并更新 g_clientfd）
bool reconnectClient(const char *ip, uint16_t port)
{
  for (int retry = 0; retry < 6; ++retry) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
      return false;
    }
    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);
    if (connect(fd, (sockaddr *)&server, sizeof(server)) == 0) {
      // 成功，更新全局 fd
      g_clientfd.store(fd);
      return true;
    }
    close(fd);
    // 指数退避策略
    std::this_thread::sleep_for(std::chrono::seconds(1 << retry));
  }
  return false;
}