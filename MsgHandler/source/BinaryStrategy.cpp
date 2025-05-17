#include "BinaryStrategy.h"
#include <iostream>
#include "JsonEncoder.h"
#include "TcpServer.h"

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
    std::string username(reinterpret_cast<const char*>(content.data() + 2), 15);
    std::cout<<"recv user avatar data "<<username<<std::endl;

    std::ofstream out(std::string("User/Avatar/").append(username), std::ios::binary);
    if (out.is_open()) {
        out.write(reinterpret_cast<char*>(content.data() + 17), content.size() - 17);
        out.close();
    } else {
        std::cerr << "无法写入" << std::endl;
    }
}