#include "JsonStrategy.h"
#include "MsgDecoder.h"

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

void LoginStrategy::execute(const int socket,const rapidjson::Document* content) const
{
    if(content->HasParseError())
    {
        std::cout<<"Doc has error : "<<content->GetParseError()<<std::endl;
        delete content;
        return;
    }
    if(content->HasMember("username")&&content->HasMember("password"))
    {
        std::string useroc=(*content)["username"].GetString();
        std::string password=(*content)["password"].GetString();
        std::cout<<"useroc "<<useroc<<"password "<<password<<std::endl;
    }
    else
    {
        
    }

}
