#include "MsgDecoder.h"

std::shared_ptr<MySqlConnPool> MsgDecoder::mysql_conn_pool_instance = nullptr;

void MsgDecoder::decode(const int socket,std::string msg)
{
	if(!mysql_conn_pool_instance)
	{
		throw std::runtime_error("Failed to decode msg, The connection pool is not initialized");
	}
	rapidjson::Document doc;
    doc.Parse(msg.c_str());
    if (doc.HasMember("type"))
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
                strategy->execute(socket, doc_content);
			}
		}
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