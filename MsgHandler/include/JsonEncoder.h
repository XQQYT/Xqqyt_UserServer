#ifndef _JSON_ENCODER_H_
#define _JSON_ENCODER_H_

#include <iostream>
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

enum class ResponseType {
    SUCCESS,
    FAIL,
    ERROR
};


enum class OKType{
    SIGNUP
};


enum class ErrorType{
    OC_CONFLICT
};



class JsonEncoder {
public:
    class OCUpdateJson{
    public:
        OCUpdateJson();
        ~OCUpdateJson();
        void addContent(const std::string oldoc,const std::string newoc);
        void getString(std::string& result);
    private:
        rapidjson::Document* doc;

    };

    class NameUpdateJson{
    public:
        NameUpdateJson();
        ~NameUpdateJson();
        void addContent(const std::string oc,const std::string newname);
        void getString(std::string& result);
    private:
        rapidjson::Document* doc;

    };


public:
    static JsonEncoder& getInstance();
    void ResponseJson(std::string& json, const ResponseType& type, const std::string& subtype);
    void DeviceCode(std::string& json, const std::string code);
    void updateFriendOC(std::string& json,const std::string& oldoc,const std::string& newoc);
    void updateFriendName(std::string& json,const std::string& oc,const std::string& newname);
public:
    void OkMsg(std::string& json,OKType type);
    void ErrorMsg(std::string& json,ErrorType type);
    void HeartBeatMsg(std::string& json);
private:
    //JsonEncoder() : buffer(), writer(buffer) {}
    JsonEncoder(){}
    JsonEncoder(const JsonEncoder&) = delete;

    ~JsonEncoder() { delete instance; } // ȷ��ɾ������ʵ��

    static JsonEncoder* instance;
};

#endif
