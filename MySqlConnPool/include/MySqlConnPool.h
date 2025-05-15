#ifndef SQLSETMANAGER_H
#define SQLSETMANAGER_H

#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <chrono>
#include <atomic>
#include "rapidjson/document.h"
#include "SqlConnSet.h"

class MySqlConnPool
{
public:
    MySqlConnPool();
    ~MySqlConnPool();
    MySqlDriver* acquire();
    void release(MySqlDriver* driver);
    void closeConnPool();
private:
    void readConf();
    void monitorFunc();
private:
    static std::string host;
    static std::string user;
    static std::string passwd;

    static int conf_min_num;
    static int conf_max_num;
    static int conf_free_time;
    static int conf_timeout;

    SqlConnSet* conn;

    std::thread* manager_thread;

    std::atomic<bool> is_running;
};



#endif // SQLSETMANAGER_H
