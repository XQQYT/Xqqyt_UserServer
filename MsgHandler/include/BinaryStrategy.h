#ifndef _BINARYSTRATEGY_H_
#define _BINARYSTRATEGY_H_

#include <fstream>
#include <cstdio>
#include <stdint.h>
#include <vector>

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
    enum class MessageType : uint16_t {
        USER_AVATAR = 0xAEAE            
    };

public:
    static BinaryStrategy* createStrategy(MessageType type);
};

#endif  //_BINARYSTRATEGY_H_
