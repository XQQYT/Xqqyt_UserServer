#ifndef SQLEXCEPTION_H
#define SQLEXCEPTION_H
#include <exception>
#include <string>
class ParaConflict : public std::exception{
    ParaConflict(const std::string& message) : msg_(message){}
    virtual const char * what() const noexcept override{
        return msg_.c_str();
    }
private:
std::string msg_;
};

#endif // SQLEXCEPTION_H
