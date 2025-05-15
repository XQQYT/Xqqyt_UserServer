#ifndef MYSQL_H
#define MYSQL_H
#include <vector>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <memory>


class MySqlDriver
{
public:
    MySqlDriver();
    //连接数据库
    void connectToSql(const std::string address,const std::string user,
                      const std::string password,const std::string dbname);
    
    void execute(std::string query,std::vector<std::string> args = {});
    
    sql::ResultSet* preExecute(std::string query,std::vector<std::string> args = {});

    void startTransaction();
    void commit();
    void disconnect();
    bool resIsEmpty();
    void freeRes();
    void resetResPos();
    void setResPos(unsigned int pos);
private:
    bool checkRes();
private:
    static sql::Driver* driver;
    sql::Connection* conn;
    sql::Statement* stmt;
    sql::PreparedStatement* pre_stmt;
    sql::ResultSet* res;
};

#endif // MYSQL_H
