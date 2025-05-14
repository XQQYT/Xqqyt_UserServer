#include "MsgDecoder.h"

void MsgDecoder::decode(const int socket,std::string msg)
{
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