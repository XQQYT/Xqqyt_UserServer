#include "BinaryStrategy.h"
#include <iostream>

BinaryStrategy* BinaryStrategyFactory::createStrategy(MessageType type)
{

    switch (type)
    {
    case MessageType::USER_AVATAR:
        return new UserAvatarStrategy();
        break;
    
    default:
        return nullptr;
        break;
    }

}

void UserAvatarStrategy::execute(const int socket,std::vector<uint8_t>  content)
{
    std::cout<<"recv user avatar data"<<std::endl;
}