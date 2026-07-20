// Adjusted include to use project include path
#include"db.h"
#include <muduo/base/Logging.h>
#include <mysql/mysql.h>
// 数据库配置信息
static std::string server{"127.0.0.1"};
static std::string user{"root"};
static std::string password{"111111"};
static std::string dbname{"chat"};
// 数据库操作类

  // 初始化数据库连接
  MySql::MySql() {
    _conn=mysql_init(nullptr);
  }
   MySql::~MySql() {
    if (_conn != nullptr) {
      mysql_close(_conn);
    }
  }
  //连接数据库
  bool  MySql::connect() {
    MYSQL *p =
        mysql_real_connect(_conn, server.c_str(), user.c_str(), password.c_str(),
                           dbname.c_str(), 3306, nullptr, 0);
    if (p != nullptr) {
      //c c++ 默认编码ACSCII,如果不设置从mysql拉下的中文：？
      mysql_query(_conn, "set names gdk");
      LOG_INFO << __FILE__ << ":" << __LINE__ << ":"<<"CONNECT SUCCESS!";
    } else
    {
      LOG_INFO << __FILE__ << ":" << __LINE__ << ":"<<"CONNECT UNSUCCESS!";
    }
    return p;
  }
  //更新
  bool  MySql::update(string sql) {
     if (mysql_query(_conn, sql.c_str())) {
      LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "更新失败"; return false;
     } 
    return true;
  }
  // 查询
   MYSQL_RES* MySql::query(string sql) {
     if (mysql_query(_conn, sql.c_str())) {
       LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "查询失败";
       return nullptr;
      }
     return mysql_use_result(_conn);
   }
  
    // 获取连接
  MYSQL* MySql::getconnection() {
    return _conn;
  }
 
