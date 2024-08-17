#ifndef SESSION_H
#define SESSION_H

#include "common.h"

#include <string.h>
#include <unistd.h>
#include <string>
#include <unordered_map>
#include <memory>

struct User{
public:
    std::string username;

    User(std::string name):username(name){}
    ~User(){}
};

const unsigned int MAGIC = 0x12345678;

struct MessageHeader{
    unsigned int magic;
    unsigned int code;
    unsigned int length;
    bool check(){//暂时只检查魔术数
        return this->magic == MAGIC;
    }
};

struct MessageInfo{
    MessageHeader *header;
    void *data;

    MessageInfo():header(nullptr), data(nullptr){}
    ~MessageInfo(){

    }

    bool check(){//暂时只检查魔术数
        return this->header->magic == MAGIC;
    }
};

enum{
    CODE_LOGIN,
    CODE_LOGOUT,
    
};

enum{
    CODE_SUCCESS,
    CODE_FAILED,
    CODE_BROADCAST,//广场广播
};

enum State{
    STAT_MAIN_MENU,//主菜单
    STAT_SQUARE,//广场聊天室
};

//存储一个连接的所有数据
//处理通信工作，发送某个东西，接收某个东西
class Session{
private:
    std::shared_ptr<User> m_user;
    bool m_isLogin;//todo: 非登录态能参与广场，只读
    int m_socket;
    State m_state;

    char m_buf[BUFSIZ];//标准输入缓冲区
public:
    Session(int socket):m_user(nullptr),m_isLogin(false), m_socket(socket),m_state(STAT_MAIN_MENU){}
    ~Session(){
        close(m_socket);
    }

    void setUser(std::string username){
        m_user = std::make_shared<User>(username);
    }
    void showMainMenu();

    //处理标准输入
    RET_CODE handleInput(){
        printf("handle input\n");
        RET_CODE ret = readInput(m_buf, BUFSIZ);
        printf("read input:%s\n", m_buf);
        if(ret == RET_ERROR){
            return RET_ERROR;
        }
        switch(m_state){
            case STAT_MAIN_MENU:
                mainMenu(m_buf);//选择选项
                break;
            case STAT_SQUARE:
                ret = chatSquare(m_buf,strlen(m_buf));
                break;
        }
        return ret;
    }

    //处理服务器的消息
    RET_CODE handleMsg(){
        printf("handle massage\n");
        MessageInfo *pmsg_info = new MessageInfo();
        RET_CODE ret =  readAndParse(pmsg_info);
        if(ret != RET_OK){
            return ret;
        }
        int command = pmsg_info->header->code;
        if(m_state == STAT_SQUARE){
            switch(command){
                case CODE_LOGIN:
                    ret = login((LoginParam*)pmsg_info->data);
                    break;
                case CODE_BROADCAST:
                    ret = handleSquareMsg((char*)pmsg_info->data, pmsg_info->header->length);
                    break;
                default://command不存在
                    return RET_ERROR;
            }
        }

        return ret;
    }



private:
    RET_CODE login(LoginParam *arg);//登录后状态变为已登录
    RET_CODE logout(){
        m_isLogin = false;
        return RET_EXIT;
    }


    RET_CODE readAndParse(MessageInfo *pmsg_info);

    RET_CODE readfd(int fd,char * buf, int buf_len);
    RET_CODE readInput(char * buf, int buf_len);
    RET_CODE readMsg(char * buf, int buf_len);

    void sendMsg(int code, const char* data, int len);


    RET_CODE mainMenu(const char * operation);

    RET_CODE chatSquare(char* msg, int msg_len){
        printf("chatting square.\n");
        sendMsg(CODE_BROADCAST, msg, msg_len);
        return RET_OK;
    }

    RET_CODE handleSquareMsg(char* msg, int len){
        printf("%s\n", msg);
        return RET_OK;
    }

};


#endif