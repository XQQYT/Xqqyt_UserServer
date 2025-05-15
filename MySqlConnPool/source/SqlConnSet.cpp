#include "SqlConnSet.h"

MySqlDriver *SqlConnSet::acquire()
{
    sem_wait(&available_num);
    mtx.lock();
    MySqlDriver* sql=sql_queue.front();
    sql_queue.pop();
    avNumDown();
    mtx.unlock();
    start=std::chrono::high_resolution_clock::now();

    return sql;
}

void SqlConnSet::release(MySqlDriver *sql)
{
    mtx.lock();
    sql_queue.push(sql);
    avNumUp();
    sql->freeRes();
    mtx.unlock();
    sem_post(&available_num);


}

void SqlConnSet::sem_release()
{
    sem_post(&available_num);
}

std::chrono::system_clock::time_point SqlConnSet::getLastUseTime()
{
    mtx.lock();
    auto tmp=start;
    mtx.unlock();
    return tmp;
}

bool SqlConnSet::isEmpty()
{
    mtx.lock();
    bool tmp=sql_queue.empty();
    mtx.unlock();
    return tmp;
}

void SqlConnSet::setDBname(std::string dbname)
{
    this->db_name=dbname;
}

std::string SqlConnSet::getDBname()
{
    return db_name;
}

int SqlConnSet::getSize()
{
    return this->size;
}

void SqlConnSet::sizeUp()
{
    size++;
}

void SqlConnSet::sizeDown()
{
    size--;
}

void SqlConnSet::avNumUp()
{
    this->av_num++;

}

void SqlConnSet::avNumDown()
{
    this->av_num--;
}

int SqlConnSet::getAv()
{
    return av_num;
}



SqlConnSet::SqlConnSet()
{
    sem_init(&available_num,0,0);
    db_name=std::string();
    this->size=0;
    av_num=0;
}

void SqlConnSet::clearSqlSet()
{
    for(int i = 0;i<sql_queue.size();i++)
    {
        auto cur_driver = acquire();
        cur_driver->disconnect();
        delete cur_driver;
        std::cout<<"delete a driver"<<std::endl;
    }
}