#include "JsonStrategy.h"
#include "MsgDecoder.h"
#include "TcpServer.h"
#include "MsgBuilder.h"
#include "OpensslHandler.h"
#include "GlobalEnum.h"
#include <filesystem>

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
    else if (type == "register_device")
	{
        return new RegisterDeviceStrategy();
	}
    else if (type == "get_user_device_list")
	{
        return new GetDeviceListStrategy();
	}
    else if(type == "update_device_comment")
    {
        return new UpdateDeviceCommentStrategy();
    }
    else if(type == "delete_device")
    {
        return new DeleteDeviceStrategy();
    }
    else if(type == "update_username")
    {
        return new UpdateUserNameStrategy();
    }
    else if(type == "update_user_password")
    {
        return new UpdateUserPasswordStrategy();
    }
	else
	{
		return nullptr;
	}
}

bool checkDocument(const rapidjson::Document& content)
{
    if(content.HasParseError())
    {
        std::cout<<"Doc has error : "<<content.GetParseError()<<std::endl;
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

void RegisterDeviceStrategy::execute(const int socket, uint8_t* key, const rapidjson::Document& content, MySqlDriver* mysql_driver) const {
    std::string response;

    do {
        if (!checkDocument(content)) break;

        if (!content.HasMember("device_name")) break;

        std::string device_name = content["device_name"].GetString();
        SocketInfo socket_info = TcpServer::getSocketInfo(socket);

        try {
            auto id_result = mysql_driver->preExecute("SELECT MAX(id) AS max_id FROM devices", {});
            uint64_t id = 0;
            if (id_result && id_result->next()) {
                id = id_result->getUInt64("max_id") + 1;
            }

            std::string code = std::to_string(encode(id));
            mysql_driver->preExecute("INSERT INTO devices(code, device_name, ip) VALUES(?, ?, ?)", 
                                     {code, device_name, std::string(socket_info.ip)});

            JsonEncoder::getInstance().DeviceCode(response, std::move(code));
            auto final_msg = MsgBuilder::getInstance().buildMsg(response, key);
            TcpServer::sendMsg(socket, *final_msg->msg);
            return;
        } catch (...) {
            break;
        }

    } while (false);

    JsonEncoder::getInstance().ResponseJson(response, ResponseType::FAIL, "register_device");

    auto final_msg = MsgBuilder::getInstance().buildMsg(response, key);
    TcpServer::sendMsg(socket, *final_msg->msg);
}

void LoginStrategy::execute(const int socket, uint8_t* key, const rapidjson::Document& content, MySqlDriver* mysql_driver) const
{
    std::string response;

    do {
        if (!checkDocument(content)) {
            break;
        }

        if (!(content.HasMember("user_name") && content.HasMember("password") && content.HasMember("device_code"))) {
            break;
        }

        std::string username = content["user_name"].GetString();
        std::string password = content["password"].GetString();
        std::string device_code = content["device_code"].GetString();
        sql::ResultSet *result;
        try{
            result = mysql_driver->preExecute("SELECT users.password FROM users WHERE user_name = ?", {username});
        }catch(...){
            break;
        }
        if (!result || !result->next()) {
            break;
        }

        if (!OpensslHandler::getInstance().verifyPassword(password, result->getString("password"))) {
            break;
        }

        JsonEncoder::getInstance().ResponseJson(response, ResponseType::SUCCESS, "login");
        try{
            mysql_driver->preExecute(
                "INSERT INTO user_device(user_name, device_code) VALUES(?, ?) "
                "ON DUPLICATE KEY UPDATE login_time = CURRENT_TIMESTAMP",
                {username, device_code}
            );
        }catch(...){
            break;
        }

        // 成功登录，发头像
        std::string avatar_path = std::string(avatar_dir).append(username);
        auto avatar_msg = MsgBuilder::getInstance().buildFile(MessageType::USER_AVATAR, username, avatar_path, key);
        TcpServer::sendMsg(socket, *avatar_msg->msg);

        auto final_msg = MsgBuilder::getInstance().buildMsg(response, key);
        TcpServer::sendMsg(socket, *final_msg->msg);
        return;
    } while (false);

    JsonEncoder::getInstance().ResponseJson(response, ResponseType::FAIL, "login");
    auto final_msg = MsgBuilder::getInstance().buildMsg(response, key);
    TcpServer::sendMsg(socket, *final_msg->msg);
}


void RegisterStrategy::execute(const int socket, uint8_t* key, const rapidjson::Document& content, MySqlDriver* mysql_driver) const
{
    std::string response;

    do {
        if (!checkDocument(content)) {
            break;
        }

        if (!(content.HasMember("user_name") && content.HasMember("password"))) {
            break;
        }

        std::string username = content["user_name"].GetString();
        std::string password = content["password"].GetString();
        std::string avatar_path = std::string(avatar_dir).append(username);

        std::shared_ptr<std::string> safe_password;
        try {
            safe_password = OpensslHandler::getInstance().dealPasswordSafe(password);
            mysql_driver->preExecute("INSERT INTO users(user_name, password, avatar_url) VALUES(? , ? , ?)",
                                     {username, *safe_password, avatar_path});
            JsonEncoder::getInstance().ResponseJson(response, ResponseType::SUCCESS, "register");
        }
        catch (...) {
            break;
        }

        auto final_msg = MsgBuilder::getInstance().buildMsg(response, key);
        TcpServer::sendMsg(socket, *final_msg->msg);
        return;

    } while (false);

    JsonEncoder::getInstance().ResponseJson(response, ResponseType::FAIL, "register");
    auto final_msg = MsgBuilder::getInstance().buildMsg(response, key);
    TcpServer::sendMsg(socket, *final_msg->msg);
}


void GetDeviceListStrategy::execute(const int socket, uint8_t* key, const rapidjson::Document& content, MySqlDriver* mysql_driver) const
{
    std::string response;

    do {
        if (!checkDocument(content)) {
            break;
        }

        if (!content.HasMember("user_name")) {
            break;
        }

        std::string username = content["user_name"].GetString();
        sql::ResultSet * result;
        try{
            result = mysql_driver->preExecute(
                "SELECT d.device_name, d.code, d.ip, ud.comment "
                "FROM user_device ud "
                "JOIN devices d ON ud.device_code = d.code "
                "WHERE ud.user_name = ?;", {username});
        }catch(...){
            break;
        }


        if (!result) {
            break;
        }

        std::vector<DeviceInfo> device_list;
        while (result->next()) {
            device_list.emplace_back(
                result->getString("device_name"),
                result->getString("code"),
                result->getString("ip"),
                result->getString("comment"));
        }

        JsonEncoder::getInstance().DeviceList(response, std::move(device_list));
        auto final_msg = MsgBuilder::getInstance().buildMsg(response, key);
        TcpServer::sendMsg(socket, *final_msg->msg);
        return;

    } while (false);

    JsonEncoder::getInstance().ResponseJson(response, ResponseType::FAIL, "get_device_list");
    auto final_msg = MsgBuilder::getInstance().buildMsg(response, key);
    TcpServer::sendMsg(socket, *final_msg->msg);
}

void UpdateDeviceCommentStrategy::execute(const int socket, uint8_t* key, const rapidjson::Document& content, MySqlDriver* mysql_driver) const {
    std::string response;

    do {
        if (!checkDocument(content)) break;

        if (!(content.HasMember("user_name") && content.HasMember("device_code") && content.HasMember("new_comment"))) break;

        std::string username = content["user_name"].GetString();
        std::string device_code = content["device_code"].GetString();
        std::string new_comment = content["new_comment"].GetString();

        try {
            mysql_driver->preExecute("UPDATE user_device SET comment = ? WHERE user_name=? AND device_code=?;", 
                                     {new_comment, username, device_code});
            JsonEncoder::getInstance().ResponseJson(response, ResponseType::SUCCESS, "update_device_comment_result");
        } catch (...) {
            break;
        }
        auto final_msg = MsgBuilder::getInstance().buildMsg(response, key);
        TcpServer::sendMsg(socket, *final_msg->msg);
        return;

    } while (false);
    JsonEncoder::getInstance().ResponseJson(response, ResponseType::FAIL, "update_device_comment_result");
    auto final_msg = MsgBuilder::getInstance().buildMsg(response, key);
    TcpServer::sendMsg(socket, *final_msg->msg);
}


void DeleteDeviceStrategy::execute(const int socket, uint8_t* key, const rapidjson::Document& content, MySqlDriver* mysql_driver) const {
    std::string response;

    do {
        if (!checkDocument(content)) break;

        if (!(content.HasMember("user_name") && content.HasMember("device_code"))) break;

        std::string username = content["user_name"].GetString();
        std::string device_code = content["device_code"].GetString();

        try {
            mysql_driver->preExecute("DELETE FROM user_device WHERE user_name=? AND device_code=?;", 
                                     {username, device_code});
            JsonEncoder::getInstance().ResponseJson(response, ResponseType::SUCCESS, "delete_device_result");
        } catch (...) {
            break;
        }
        auto final_msg = MsgBuilder::getInstance().buildMsg(response, key);
        TcpServer::sendMsg(socket, *final_msg->msg);
        return;
    } while (false);

    JsonEncoder::getInstance().ResponseJson(response, ResponseType::FAIL, "delete_device_result");
    auto final_msg = MsgBuilder::getInstance().buildMsg(response, key);
    TcpServer::sendMsg(socket, *final_msg->msg);
}


void UpdateUserNameStrategy::execute(const int socket, uint8_t* key, const rapidjson::Document& content, MySqlDriver* mysql_driver) const {
    std::string response;

    do {
        if (!checkDocument(content)) break;

        if (!(content.HasMember("user_name") && content.HasMember("new_user_name"))) break;

        std::string username = content["user_name"].GetString();
        std::string new_user_name = content["new_user_name"].GetString();

        std::string old_avatar_path = "User/Avatar/" + username;
        std::string new_avatar_path = "User/Avatar/" + new_user_name;

        try {
            std::filesystem::rename(old_avatar_path, new_avatar_path);
            mysql_driver->preExecute("UPDATE users SET user_name = ? WHERE user_name=?;", {new_user_name, username});
            JsonEncoder::getInstance().ResponseJson(response, ResponseType::SUCCESS, "update_user_name");
        } catch (...) {
            // 回滚头像更名和用户名更名尝试
            try {
                std::filesystem::rename(new_avatar_path, old_avatar_path);
                mysql_driver->preExecute("UPDATE users SET user_name = ? WHERE user_name=?;", {username, new_user_name});
            } catch (...) {}
            break;
        }
        auto final_msg = MsgBuilder::getInstance().buildMsg(response, key);
        TcpServer::sendMsg(socket, *final_msg->msg);
        return;
    } while (false);

    JsonEncoder::getInstance().ResponseJson(response, ResponseType::FAIL, "update_user_name");
    auto final_msg = MsgBuilder::getInstance().buildMsg(response, key);
    TcpServer::sendMsg(socket, *final_msg->msg);
}


void UpdateUserPasswordStrategy::execute(const int socket, uint8_t* key, const rapidjson::Document& content, MySqlDriver* mysql_driver) const {
    std::string response;

    do {
        if (!checkDocument(content)) break;

        if (!(content.HasMember("user_name") && content.HasMember("old_password") && content.HasMember("new_password"))) break;

        std::string username = content["user_name"].GetString();
        std::string old_password = content["old_password"].GetString();
        std::string new_password = content["new_password"].GetString();

        sql::ResultSet * result;
        try{
            result = mysql_driver->preExecute("SELECT users.password FROM users WHERE user_name = ?", {username});
        }
        catch(...){
            break;
        }
        
        if (!result || !result->next()) break;

        if (!OpensslHandler::getInstance().verifyPassword(old_password, result->getString("password"))) break;

        try {
            auto safe_password = OpensslHandler::getInstance().dealPasswordSafe(new_password);
            mysql_driver->preExecute("UPDATE users SET password = ? WHERE user_name=?;", {*safe_password, username});
            JsonEncoder::getInstance().ResponseJson(response, ResponseType::SUCCESS, "update_user_password");
        } catch (...) {
            break;
        }
        auto final_msg = MsgBuilder::getInstance().buildMsg(response, key);
        TcpServer::sendMsg(socket, *final_msg->msg);
        return;
    } while (false);

    if (response.empty())
        JsonEncoder::getInstance().ResponseJson(response, ResponseType::FAIL, "update_user_password");

    auto final_msg = MsgBuilder::getInstance().buildMsg(response, key);
    TcpServer::sendMsg(socket, *final_msg->msg);
}
