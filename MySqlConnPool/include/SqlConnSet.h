#ifndef SQLCONNSET_H
#define SQLCONNSET_H

#include "MySqlDriver.h"
#include <queue>
#include <chrono>
#include <semaphore.h>
#include <mutex>
class SqlConnSet
{
public:
    SqlConnSet();
    void clearSqlSet();
    MySqlDriver* acquire();
    void release(MySqlDriver* sql);
    void sem_release();
    std::chrono::high_resolution_clock::time_point getLastUseTime();
    bool isEmpty();
    void setDBname(std::string dbname);
    std::string getDBname();

    int getSize();
    void sizeUp();
    void sizeDown();

    void avNumUp();
    void avNumDown();
    int getAv();
private:

    std::mutex mtx;

    std::string db_name;

    std::queue<MySqlDriver*> sql_queue;

    sem_t available_num;

    std::chrono::high_resolution_clock::time_point start;

    int size;

    int av_num;


};


#endif // MYSQLCONNPOOL_H
