#include "MySqlConnPool.h"
#include <functional>

std::string MySqlConnPool::host=std::string();
std::string MySqlConnPool::user=std::string();
std::string MySqlConnPool::passwd=std::string();
int MySqlConnPool::conf_min_num=0;
int MySqlConnPool::conf_max_num=0;
int MySqlConnPool::conf_free_time=0;
int MySqlConnPool::conf_timeout=0;

MySqlConnPool::MySqlConnPool():conn(new SqlConnSet)
,is_running(true)
{
    readConf();
    conn->setDBname("xqqyt_user");
    manager_thread=new std::thread(std::bind(&MySqlConnPool::monitorFunc,this));
}

MySqlConnPool::~MySqlConnPool()
{
    closeConnPool();
    if(manager_thread->joinable())
    {
        manager_thread->join();
    }
    std::cout<<"close conn pool done"<<std::endl;
}

void MySqlConnPool::readConf()
{
    std::ifstream conf_file;
    conf_file.open("MySqlConnPool.json",std::ios::in);
    if(!conf_file.is_open())
    {
        std::cout<<"open file failed"<<std::endl;
    }
    std::string line,content;
    while(conf_file>>line)
    {
        content.append(line);
    }

    rapidjson::Document doc;
    doc.Parse(content.c_str());

    if (!doc.IsObject() || !doc.HasMember("host")) 
    {
        throw std::runtime_error("Invalid MySqlConnPool.conf");
    }

    host=doc["host"].GetString();
    user=doc["user"].GetString();
    passwd=doc["passwd"].GetString();
    conf_min_num=doc["min_conn_num"].GetInt();
    conf_max_num=doc["max_conn_num"].GetInt();
    conf_free_time=doc["free_time"].GetInt();

}

void MySqlConnPool::monitorFunc()
{
    std::list<SqlConnSet*>::iterator it;
    while(is_running)
    {
        auto cur_time=std::chrono::high_resolution_clock::now();
        auto last_time=conn->getLastUseTime();
        auto free_time=std::chrono::duration_cast<std::chrono::milliseconds>(cur_time-last_time);
        int free_time_int=static_cast<int>(free_time.count());
        if(free_time_int>=this->conf_free_time
                &&conn->getSize()>conf_min_num
                &&conn->getAv()>2
                )
        {
            for(int i=0;i<1;i++)
            {
                auto dis_sql=conn->acquire();
                dis_sql->disconnect();
                conn->sizeDown();
            }
            std::cout<<conn->getDBname()<<"  size -> "<<conn->getSize()<<
                    "available -> "<<conn->getAv()<<std::endl;
        }
        else if(conn->getSize()<conf_max_num
                &&conn->getAv()<2
                )
        {
            for(int i=0;i<1;i++)
            {
                MySqlDriver* new_sql=new MySqlDriver;
                new_sql->connectToSql(host,user,passwd,conn->getDBname());
                conn->release(new_sql);
                conn->sizeUp();
            }
            std::cout<<conn->getDBname()<<"  size -> "<<conn->getSize()<<
                    "available -> "<<conn->getAv()<<std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void MySqlConnPool::closeConnPool()
{
    is_running = false;
    conn->clearSqlSet();
}

MySqlDriver* MySqlConnPool::acquire()
{
    return conn->acquire();
}
void MySqlConnPool::release(MySqlDriver* driver)
{
    conn->release(driver);
}