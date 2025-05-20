#ifndef _JSONSTRATEGY_H_
#define _JSONSTRATEGY_H_

#include "JsonEncoder.h"
#include "MySqlConnPool.h"
#include <fstream>
#include <cstdio>

class JsonStrategy {
public:
    virtual void execute(const int socket, uint8_t* key, const rapidjson::Document* content, MySqlDriver* mysql_driver) const = 0;
    virtual ~JsonStrategy(){
    }

};

class RegisterDeviceStrategy : public JsonStrategy{
public:
    void execute(const int socket, uint8_t* key, const rapidjson::Document* content, MySqlDriver* mysql_driver) const override;
    ~RegisterDeviceStrategy(){}
};

class LoginStrategy : public JsonStrategy{
public:
    void execute(const int socket, uint8_t* key, const rapidjson::Document* content, MySqlDriver* mysql_driver) const override;
    ~LoginStrategy(){}
};

class RegisterStrategy : public JsonStrategy{
public:
    void execute(const int socket, uint8_t* key, const rapidjson::Document* content, MySqlDriver* mysql_driver) const override;
    ~RegisterStrategy(){}
};

class GetDeviceListStrategy : public JsonStrategy{
public:
    void execute(const int socket, uint8_t* key, const rapidjson::Document* content, MySqlDriver* mysql_driver) const override;
    ~GetDeviceListStrategy(){}
};

class UpdateDeviceCommentStrategy : public JsonStrategy{
public:
    void execute(const int socket, uint8_t* key, const rapidjson::Document* content, MySqlDriver* mysql_driver) const override;
    ~UpdateDeviceCommentStrategy(){}
};

class DeleteDeviceStrategy : public JsonStrategy{
public:
    void execute(const int socket, uint8_t* key, const rapidjson::Document* content, MySqlDriver* mysql_driver) const override;
    ~DeleteDeviceStrategy(){}
};

class UpdateUserNameStrategy : public JsonStrategy{
public:
    void execute(const int socket, uint8_t* key, const rapidjson::Document* content, MySqlDriver* mysql_driver) const override;
    ~UpdateUserNameStrategy(){}
};

class UpdateUserPasswordStrategy : public JsonStrategy{
    public:
        void execute(const int socket, uint8_t* key, const rapidjson::Document* content, MySqlDriver* mysql_driver) const override;
        ~UpdateUserPasswordStrategy(){}
    };
    

class JsonStrategyFactory {
public:
    static JsonStrategy* createStrategy(const std::string& type);
};

#endif
