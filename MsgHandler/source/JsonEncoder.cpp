#include "JsonEncoder.h"

JsonEncoder* JsonEncoder::instance = nullptr;

JsonEncoder& JsonEncoder::getInstance()
{
	if (instance == nullptr)
	{
		instance = new JsonEncoder();

	}
	return *instance;
}

void JsonEncoder::ResponseJson(std::string& json, const ResponseType& type, const std::string& subtype)
{
	rapidjson::Document doc;
	doc.SetObject();

	auto& doc_allocator = doc.GetAllocator();
	std::string type_str;
	switch (type) {
	case ResponseType::SUCCESS:
		type_str = "success";
		break;
	case ResponseType::FAIL:
		type_str = "fail";
		break;
	case ResponseType::ERROR:
		type_str = "error";
		break;
	default:
		throw std::runtime_error("Invalid LogInType");
	}
	doc.AddMember("type", rapidjson::StringRef("respond"), doc_allocator);

	doc.AddMember("content", rapidjson::Value().SetObject(), doc_allocator);

	doc["content"].AddMember("status", rapidjson::StringRef(type_str.c_str()), doc_allocator);
	doc["content"].AddMember("subtype", rapidjson::StringRef(subtype.c_str()), doc_allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

	doc.Accept(writer);
    json = buffer.GetString();
	buffer.Clear();
	writer.Reset(buffer);
}


void JsonEncoder::DeviceCode(std::string& json, const std::string code)
{
	rapidjson::Document doc;
	doc.SetObject();

	auto& doc_allocator = doc.GetAllocator();


	doc.AddMember("type", rapidjson::StringRef("device_code"), doc_allocator);

	doc.AddMember("content", rapidjson::Value().SetObject(), doc_allocator);

    doc["content"].AddMember("device_code", rapidjson::StringRef(code.c_str()), doc_allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

	doc.Accept(writer);

	json = std::move(buffer.GetString());
	buffer.Clear();
    writer.Reset(buffer);
}

void JsonEncoder::LastestVersion(std::string& json, const std::string& version, const std::string& description, const std::string& date)
{
	rapidjson::Document doc;
	doc.SetObject();

	auto& doc_allocator = doc.GetAllocator();


	doc.AddMember("type", rapidjson::StringRef("lastest_version"), doc_allocator);

	doc.AddMember("content", rapidjson::Value().SetObject(), doc_allocator);

    doc["content"].AddMember("version", rapidjson::StringRef(version.c_str()), doc_allocator);
    doc["content"].AddMember("description", rapidjson::StringRef(description.c_str()), doc_allocator);
    doc["content"].AddMember("date", rapidjson::StringRef(date.c_str()), doc_allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

	doc.Accept(writer);

	json = std::move(buffer.GetString());
	buffer.Clear();
    writer.Reset(buffer);
}

void JsonEncoder::DeviceList(std::string& json, const std::vector<DeviceInfo> device_list)
{
    rapidjson::Document doc;
    doc.SetObject();
    auto& allocator = doc.GetAllocator();

    doc.AddMember("type", rapidjson::StringRef("device_list"), allocator);

    rapidjson::Value content(rapidjson::kObjectType);
    rapidjson::Value deviceArray(rapidjson::kArrayType);

    for (const auto& device : device_list)
    {
        rapidjson::Value deviceObj(rapidjson::kObjectType);

        deviceObj.AddMember("device_name", rapidjson::Value(device.device_name.c_str(), allocator), allocator);
        deviceObj.AddMember("code",        rapidjson::Value(device.code.c_str(), allocator), allocator);
        deviceObj.AddMember("ip",          rapidjson::Value(device.ip.c_str(), allocator), allocator);
        deviceObj.AddMember("comment",     rapidjson::Value(device.comment.c_str(), allocator), allocator);

        deviceArray.PushBack(deviceObj, allocator);
    }

    content.AddMember("device_list", deviceArray, allocator);
    doc.AddMember("content", content, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    json = std::move(buffer.GetString());
    buffer.Clear();
    writer.Reset(buffer);
}

void JsonEncoder::ErrorMsg(std::string &json, ErrorType type)
{
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    auto& doc_allocator = doc.GetAllocator();

    switch (type) {
    case ErrorType::OC_CONFLICT:
        doc.AddMember("type", rapidjson::StringRef("oc_conflict"), doc_allocator);
        break;
    default:
        buffer.Clear();
        writer.Reset(buffer);
        return;

    }

    doc.AddMember("content", rapidjson::Value().SetObject(), doc_allocator);

    doc.Accept(writer);
    json=std::move(buffer.GetString());
    buffer.Clear();
    writer.Reset(buffer);
}

void JsonEncoder::HeartBeatMsg(std::string &json)
{
    rapidjson::Document doc;
    doc.SetObject();

    auto& doc_allocator = doc.GetAllocator();

    doc.AddMember("type", rapidjson::StringRef("heartbeat"), doc_allocator);

    doc.AddMember("content", rapidjson::Value().SetObject(), doc_allocator);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    doc.Accept(writer);
    json=std::move(buffer.GetString());
    buffer.Clear();
    writer.Reset(buffer);
}



JsonEncoder::OCUpdateJson::OCUpdateJson()
{
    doc=new rapidjson::Document;
    doc->SetObject();

    auto& doc_allocator=doc->GetAllocator();
    doc->AddMember("type",rapidjson::StringRef("ocupdatetmp"),doc_allocator);
    rapidjson::Value arrayobj(rapidjson::kArrayType);
    doc->AddMember("array",arrayobj,doc_allocator);
}

JsonEncoder::OCUpdateJson::~OCUpdateJson()
{
    delete doc;
}

void JsonEncoder::OCUpdateJson::addContent(const std::string oldoc, const std::string newoc)
{
    std::cout<<oldoc<<"   "<<newoc<<std::endl;
    auto& doc_allocator=doc->GetAllocator();

    rapidjson::Value obj(rapidjson::kObjectType);

    obj.AddMember("oldoc",rapidjson::Value().SetString(oldoc.c_str(),doc_allocator),doc_allocator);
    obj.AddMember("newoc",rapidjson::Value().SetString(newoc.c_str(),doc_allocator),doc_allocator);
    (*doc)["array"].PushBack(obj,doc_allocator);

}

void JsonEncoder::OCUpdateJson::getString(std::string& result)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc->Accept(writer);
    result=std::move(buffer.GetString());
    buffer.Clear();
    writer.Reset(buffer);
}





JsonEncoder::NameUpdateJson::NameUpdateJson()
{
    doc=new rapidjson::Document;
    doc->SetObject();

    auto& doc_allocator=doc->GetAllocator();
    doc->AddMember("type",rapidjson::StringRef("nameupdatetmp"),doc_allocator);
    rapidjson::Value arrayobj(rapidjson::kArrayType);
    doc->AddMember("array",arrayobj,doc_allocator);
}

JsonEncoder::NameUpdateJson::~NameUpdateJson()
{
    delete doc;
}


void JsonEncoder::NameUpdateJson::addContent(const std::string oc, const std::string newname)
{
    auto& doc_allocator=doc->GetAllocator();

    rapidjson::Value obj(rapidjson::kObjectType);

    obj.AddMember("oc",rapidjson::Value().SetString(oc.c_str(),doc_allocator),doc_allocator);
    obj.AddMember("newname",rapidjson::Value().SetString(newname.c_str(),doc_allocator),doc_allocator);
    (*doc)["array"].PushBack(obj,doc_allocator);
}

void JsonEncoder::NameUpdateJson::getString(std::string &result)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc->Accept(writer);
    result=std::move(buffer.GetString());
    buffer.Clear();
    writer.Reset(buffer);
}
