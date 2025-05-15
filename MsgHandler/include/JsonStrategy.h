#ifndef _JSONSTRATEGY_H_
#define _JSONSTRATEGY_H_

#include "JsonEncoder.h"
#include <fstream>
#include <cstdio>

class JsonStrategy {
public:
    virtual void execute(const int socket,const rapidjson::Document* content) const = 0;
    virtual ~JsonStrategy(){
    }

};

class LoginStrategy : public JsonStrategy{
public:
    void execute(const int socket,const rapidjson::Document* content) const override;
    ~LoginStrategy(){}
};


class JsonStrategyFactory {
public:
    static JsonStrategy* createStrategy(const std::string& type);
};

#endif
