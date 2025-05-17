#ifndef _BINARYSTRATEGY_H_
#define _BINARYSTRATEGY_H_

#include <fstream>
#include <cstdio>
#include <stdint.h>
#include <vector>
#include "GlobalEnum.h"

class BinaryStrategy {
public:
    virtual void execute(const int socket,std::vector<uint8_t> content) = 0;
    virtual ~BinaryStrategy(){
    }

};

class UserAvatarStrategy : public BinaryStrategy{
public:
    void execute(const int socket,std::vector<uint8_t>  content) override;
    ~UserAvatarStrategy(){}
};


class BinaryStrategyFactory {
public:
    static BinaryStrategy* createStrategy(MessageType type);
};

#endif  //_BINARYSTRATEGY_H_
