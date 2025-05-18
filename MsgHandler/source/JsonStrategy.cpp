#include "JsonStrategy.h"
#include "MsgDecoder.h"
#include "TcpServer.h"
#include "MsgBuilder.h"
#include "OpensslHandler.h"

static const char* avatar_dir = "User/Avatar/";

JsonStrategy* JsonStrategyFactory::createStrategy(const std::string& type)
{

	if (type == "login")
	{
        return new LoginStrategy();
	}
    else if(type == "register")
    {
        return new RegisterStrategy();
    }
    if (type == "register_device")
	{
        return new RegisterDeviceStrategy();
	}
	else
	{
		return nullptr;
	}
}

bool checkDocument(const rapidjson::Document* content)
{
    if(content->HasParseError())
    {
        std::cout<<"Doc has error : "<<content->GetParseError()<<std::endl;
        return false;
    }
    return true;
}

uint32_t encode(uint32_t id) {
    uint32_t key = 0xA5A5A5A5; // 固定密钥
    uint32_t prime = 49999;    // 大质数

    return ((id ^ key) * prime) % 1000000000; // 保证最大9位
}

uint32_t decode(uint32_t code) {
    uint32_t prime = 49999;
    uint32_t inv_prime = 588625789; // 49999 的模 10^9 逆元 (precomputed)
    uint32_t key = 0xA5A5A5A5;

    return (code * inv_prime % 1000000000) ^ key;
}

void RegisterDeviceStrategy::execute(const int socket, uint8_t* key, const rapidjson::Document* content, MySqlDriver* mysql_driver) const
{
    if(!checkDocument(content))
    {
        delete content;
        return;
    }
    if(content->HasMember("device_name"))
    {
        std::string device_name = (*content)["device_name"].GetString();
        SocketInfo socket_info = TcpServer::getSocketInfo(socket);

        uint64_t id = static_cast<uint64_t>(mysql_driver->getLastInsertId()) + 1;
        std::string code = std::to_string(encode(id));
        mysql_driver->preExecute("INSERT INTO devices(code,device_name,ip) VALUES(?,?,?)",{code,device_name,std::string(socket_info.ip)});
        std::string response;
        JsonEncoder::getInstance().DeviceCode(response, std::move(code));
        auto final_msg = MsgBuilder::getInstance().buildMsg(response,key);
        TcpServer::sendMsg(socket, *final_msg->msg);
    }
}

void LoginStrategy::execute(const int socket, uint8_t* key, const rapidjson::Document* content, MySqlDriver* mysql_driver) const
{

    if(!checkDocument(content))
    {
        delete content;
        return;
    }
    if(content->HasMember("user_name")&&content->HasMember("password")&&content->HasMember("device_code"))
    {
        std::string username = (*content)["user_name"].GetString();
        std::string password = (*content)["password"].GetString();
        std::string device_code = (*content)["device_code"].GetString();

        auto safe_password = OpensslHandler::getInstance().dealPasswordSafe(password);

        std::cout<<"login safe password "<<*safe_password<<std::endl;

        auto result = mysql_driver->preExecute("SELECT users.password FROM users WHERE username = ?",{username});

        std::string response;
        if(result)
        {
            result->next();
            bool status = OpensslHandler::getInstance().verifyPassword(password, result->getString("password"));
            if(status)
            {
                JsonEncoder::getInstance().loginJson(response,LogInType::PASS);
                mysql_driver->preExecute(
                    "INSERT INTO user_device(user_name, device_code) VALUES(?, ?) "
                    "ON DUPLICATE KEY UPDATE login_time = CURRENT_TIMESTAMP",
                    {username, device_code}
                );
            }
            else
            {
                JsonEncoder::getInstance().loginJson(response,LogInType::ERROR);
            }
        }
        else
        {   
            JsonEncoder::getInstance().loginJson(response,LogInType::ERROR);
        }

        auto final_msg = MsgBuilder::getInstance().buildMsg(response,key);
        TcpServer::sendMsg(socket, *final_msg->msg);
    }
    else
    {
        
    }

}

void RegisterStrategy::execute(const int socket, uint8_t* key, const rapidjson::Document* content, MySqlDriver* mysql_driver) const
{

    if(!checkDocument(content))
    {
        delete content;
        return;
    }
    if(content->HasMember("user_name")&&content->HasMember("password"))
    {
        std::string username = (*content)["user_name"].GetString();
        std::string password = (*content)["password"].GetString();
        std::string avatar_path = std::string(avatar_dir).append(username);

        auto safe_password = OpensslHandler::getInstance().dealPasswordSafe(password);

        std::cout<<"register safe password "<<*safe_password<<std::endl;

        mysql_driver->preExecute("INSERT INTO users(username, password, avatar_url) VALUES(? , ? , ?)", {username, *safe_password, avatar_path});
    }
    else
    {
        
    }

}
