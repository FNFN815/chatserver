#include "chatserver.hpp"
#include "chatservice.hpp"
#include <csignal>
#include <iostream>
#include<signal.h>
using namespace std;
//处理CTRL+C异常退出，重置user状态state
void resetHandler(int) {
  ChatService::instance()->reset();
  exit(0);
}
int main(int argc,char** argv) {
   if (argc < 3) {
    cout << "command invalid ! example: ./chatserver 127.0.0.1 6000" << endl;
    exit(-1);
  }
  // 通过解析命令行参数传递ip , port
  char *ip = argv[1];
  uint16_t port = atoi(argv[2]);
  signal(SIGINT,resetHandler);
  EventLoop loop;
  InetAddress addr(ip, port);
  ChatServer chatserver(&loop, addr, "myserver");
  chatserver.start();
  loop.loop();
  return 0;
}