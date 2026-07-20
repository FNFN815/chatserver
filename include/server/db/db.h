#ifndef DB_H
#define DB_H

#include<mysql/mysql.h>
#include <muduo/base/Logging.h>

using namespace muduo;
using namespace std;


// 数据库操作类
class MySql{
public:
  // 初始化数据库连接
  MySql() ;
  ~MySql() ;
  //连接数据库
  bool connect();
  //更新
  bool update(string sql);
  //查询
  MYSQL_RES *query(string sql);
  // 获取连接
  MYSQL *getconnection();

private:
  MYSQL* _conn;
  
};
#endif