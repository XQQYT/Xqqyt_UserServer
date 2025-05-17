#include "MsgDecoder.h"

std::shared_ptr<MySqlConnPool> MsgDecoder::mysql_conn_pool_instance = nullptr;

void MsgDecoder::decode(const int socket,std::vector<uint8_t> msg_vec , bool is_binary, uint8_t* key)
{
	if(!mysql_conn_pool_instance)
	{
		throw std::runtime_error("Failed to decode msg, The connection pool is not initialized");
	}
	if(!is_binary)
	{
		std::string msg(reinterpret_cast<const char*>(msg_vec.data()), msg_vec.size());
		std::cout<<msg<<std::endl;
		rapidjson::Document doc;
		doc.Parse(msg.c_str());
		if (doc.HasParseError() || !doc.IsObject()) {
			std::cerr << "Invalid JSON data received." << std::endl;
			return;
		}
		if (doc.HasMember("type") )
		{
			std::string type = doc["type"].GetString();
			if (doc.HasMember("content") && doc["content"].IsObject())
			{
				rapidjson::Value& json_content = doc["content"].GetObject();
				rapidjson::Document* doc_content=new rapidjson::Document;
				doc_content->CopyFrom(json_content,doc_content->GetAllocator());
				auto strategy= JsonStrategyFactory::createStrategy(type);
				if (strategy != nullptr)
				{
					strategy->execute(socket, key, doc_content);
				}
				delete strategy;
			}
		}
	}
	else
	{
		uint16_t type;
        memcpy(&type, msg_vec.data(), 2);
        auto strategy = BinaryStrategyFactory::createStrategy(static_cast<MessageType>(type));
		if (strategy != nullptr)
		{
			//注意传入的数组并没有去除type 的2字节
			strategy->execute(socket, std::move(msg_vec));
		}
		delete strategy;
	}
}

void MsgDecoder::setMySqlConnPool(std::shared_ptr<MySqlConnPool> instance)
{
	mysql_conn_pool_instance = instance;
}

std::shared_ptr<MySqlConnPool> MsgDecoder::getMySqlConnPool()
{
	return mysql_conn_pool_instance;
}