#ifndef SESSION_H
#define SESSION_H

#include <string>

struct User{
public:
    std::string username;
    int fd;

    User(std::string name, int fd):username(name), fd(fd){}
    ~User(){}
};

const unsigned int MAGIC = 0x12345678;

struct MessageHeader{
    unsigned int magic;
    unsigned int code;
    unsigned int length;
    void * data;
};

enum{
    CODE_LOGIN,
    CODE_LOGOUT,
    
};

//存储一个连接的所有数据
//处理一个连接所有可能发生的操作
class Session{
private:
    User *m_user;
    bool m_isLogin;//todo: 非登录态能参与广场，只读
    int m_socket;
public:
    Session(int socket):m_user(nullptr),m_isLogin(false), m_socket(socket){}
    ~Session(){}

    int handleMsg();

private:
    int login();//登录后状态变为已登录
    int logout();

    int parseMsg(){
        
    }
};


#endif