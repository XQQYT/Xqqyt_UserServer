#include "MySqlDriver.h"

sql::Driver* MySqlDriver::driver=nullptr;

MySqlDriver::MySqlDriver()
{
    if(driver==nullptr)
    {
        driver=sql::mysql::get_driver_instance();
    }
}

void MySqlDriver::connectToSql(const std::string address, const std::string user, const std::string password, const std::string dbname)
{
    conn=driver->connect(address,user,password);
    conn->setSchema(dbname);
    stmt=conn->createStatement();
}

void MySqlDriver::execute(std::string query,std::vector<std::string> args)
{
    stmt->execute(query);
}

sql::ResultSet* MySqlDriver::preExecute(std::string query,std::vector<std::string> args)
{
    pre_stmt=conn->prepareStatement(query);
    int size=args.size();
    for(int i=0;i<size;i++)
    {
        pre_stmt->setString(i+1,args[i]);
    }
    sql::ResultSet* res=pre_stmt->executeQuery();
    if(checkRes())
    {
        res = nullptr;
        freeRes();
    }
    return res;
}

void MySqlDriver::startTransaction()
{
    conn->setAutoCommit(false);
}

void MySqlDriver::commit()
{
    conn->commit();
    conn->setAutoCommit(true);
}

void MySqlDriver::disconnect()
{
    conn->close();
    delete stmt;
}

bool MySqlDriver::resIsEmpty()
{
    return checkRes();
}

void MySqlDriver::freeRes()
{
    if(res)
        delete res;
    res=nullptr;
}

void MySqlDriver::resetResPos()
{
    res->absolute(1);
}

void MySqlDriver::setResPos(unsigned int pos)
{
    res->absolute(pos);
}

bool MySqlDriver::checkRes()
{
    if(!res) return false;
    bool is_valid = res->absolute(1);
    res->beforeFirst();
    return is_valid;
}

int64_t MySqlDriver::getLastInsertId()
{
    if (!stmt) {
        stmt = conn->createStatement();
    }

    if (res) {
        delete res;
        res = nullptr;
    }

    res = stmt->executeQuery("SELECT MAX(id) FROM devices");
    if (res && res->next()) {
        return res->getInt64(1);
    }
    return -1;
}