#ifndef COMMON_H
#define COMMON_H
#include <string>

struct User{
public:
    std::string username;
    int fd;

    User(std::string name, int fd):username(name), fd(fd){}
    ~User(){}
};

#endif