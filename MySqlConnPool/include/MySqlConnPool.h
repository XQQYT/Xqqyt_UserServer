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

class MySqlConnGuardPtr {
public:
    explicit MySqlConnGuardPtr(std::shared_ptr<MySqlConnPool> pool)
        : pool_(std::move(pool)), conn_(pool_->acquire()) {}
     ~MySqlConnGuardPtr() {
        std::cout<<"release a conn"<<std::endl;
        if (conn_) pool_->release(conn_);
    }
    MySqlConnGuardPtr(const MySqlConnGuardPtr&) = delete;
    MySqlConnGuardPtr& operator=(const MySqlConnGuardPtr&) = delete;
    MySqlDriver* get() const {
        if (!conn_) throw std::runtime_error("Null DB connection");
        return conn_;
    }
    bool valid() const { return conn_ != nullptr; }
    MySqlDriver* operator->() const { return get(); }
private:
    std::shared_ptr<MySqlConnPool> pool_;
    MySqlDriver* conn_;
};


#endif // SQLSETMANAGER_H
