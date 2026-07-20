#ifndef REDIS_H
#define REDIS_H
#include <hiredis/hiredis.h>
#include <functional>
#include <thread>
using namespace std;
class Redis {
public:
  // 构造与析构
  Redis();
  ~Redis();
  // 连接redis服务器
  bool connect();
  // 向redis指定通道channel发布消息
  bool publish(int channel, string message);
   // 向redis指定通道channel订阅消息
  bool subscribe(int channel);
   // 向redis指定通道channel取消订阅消息
  bool unsubcribe(int channel);
  // 接收独立线程订阅的通道的消息
  void observer_channel_message();
  // 初始化向业务层上报通道消息的回调对象
  void init_notify_handler(function<void (int,string)> fn);
private:
  // hiredis 同步上下文对象，负责publish
    redisContext* _publish_context;
    // hiredis 同步上下文对象，负责subcribe
    redisContext *_subscribe_context;
    // 回调操作，收到订阅消息向service上报
    function<void(int,string)> _notify_message_handler; 
};


#endif