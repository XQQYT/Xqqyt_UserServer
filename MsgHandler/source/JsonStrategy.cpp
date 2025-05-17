#include "JsonStrategy.h"
#include "MsgDecoder.h"
#include "TcpServer.h"
#include "MsgBuilder.h"

JsonStrategy* JsonStrategyFactory::createStrategy(const std::string& type)
{

	if (type == "login")
	{
        return new LoginStrategy();
	}
	else
	{
		return nullptr;
	}
}

void LoginStrategy::execute(const int socket, uint8_t* key, const rapidjson::Document* content) const
{
    if(content->HasParseError())
    {
        std::cout<<"Doc has error : "<<content->GetParseError()<<std::endl;
        delete content;
        return;
    }
    if(content->HasMember("user_name")&&content->HasMember("password"))
    {
        std::string useroc=(*content)["user_name"].GetString();
        std::string password=(*content)["password"].GetString();
        std::cout<<"useroc "<<useroc<<"password "<<password<<std::endl;

        std::string response;
        JsonEncoder::getInstance().loginJson(response,LogInType::PASS);
        auto final_msg = MsgBuilder::getInstance().buildMsg(response,key);
        TcpServer::sendMsg(socket, *final_msg->msg);
    }
    else
    {
        
    }

}
