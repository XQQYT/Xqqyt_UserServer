#include "MsgDecoder.h"

void MsgDecoder::decode(const int socket,std::vector<uint8_t>& msg_vec , bool is_binary, uint8_t* key, std::shared_ptr<MySqlConnPool> conn_pool)
{

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
				rapidjson::Document doc_content;
				doc_content.CopyFrom(json_content, doc_content.GetAllocator());

				auto strategy = JsonStrategyFactory::createStrategy(type);
				if (strategy) {
					auto conn_guard = MySqlConnGuardPtr(conn_pool);
					if (!conn_guard.valid()) return;
					strategy->execute(socket, key, doc_content, conn_guard.get());
					delete strategy;
				}
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
			strategy->execute(socket, key, std::move(msg_vec));
		}
		delete strategy;
	}
}