#ifndef SESSION_H
#define SESSION_H

#include "database.h"
#include "common.h"

#include "lock.h"

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

using UserMap = std::unordered_map<int, User>;
extern std::shared_ptr<UserMap> g_users;//当前活动用户
extern Lock g_mtx;


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


class SessionMng;
//存储一个连接的所有数据
//处理一个连接所有可能发生的操作
class Session{
private:
    std::shared_ptr<User> m_user;
    bool m_isLogin;//todo: 非登录态能参与广场，只读
    int m_socket;
public:
    Session(int socket):m_user(nullptr),m_isLogin(false), m_socket(socket){}
    ~Session(){
        printf("client %d exited.\n", m_socket);
        close(m_socket);
    }

    void setUser(std::string username){
        std::shared_ptr<UserMap> tmp;
        {
            tmp = g_users;
        }
        tmp->insert(std::pair<int, User>(m_socket, std::move(User(username))));
        m_user = std::make_shared<User>(username);
    }

    RET_CODE handleMsg(){
        MessageInfo *pmsg_info = new MessageInfo();
        RET_CODE ret =  readAndParse(pmsg_info);
        if(ret == RET_EXIT){
            ret = logout();
        }
        if(ret != RET_OK){
            return ret;
        }
        
        int command = pmsg_info->header->code;
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
        return ret;
    }

    RET_CODE broadcastMsg(const char *buf, int len){//广播
        // printf("broadcasting...\n");
        //添加用户信息
        return sendToSquare(buf, len);
    }

private:
    RET_CODE login(LoginParam *arg){//登录后状态变为已登录
        LoginParam * plogin = (LoginParam*)arg;
        if(!DataBase::getInstance()->login(plogin)){
            return RET_ERROR;
        }
        //如果登录成功
        m_isLogin = true;
        //todo:发送给客户端一些信息
        sendMsg(CODE_SUCCESS, nullptr, 0);
        //todo:欢迎语
        return RET_OK;

    }
    RET_CODE logout(){
        m_isLogin = false;
        return RET_EXIT;
    }

    //广场，是一个大聊天室
    RET_CODE handleSquareMsg(char* msg, int len){
        int buf_len = len + 100;
        char *buf = new char[buf_len];
        memset(buf, 0, buf_len);
        snprintf(buf, buf_len-1, ">> %s: %s", m_user->username.c_str(), msg);

        //然后广播
        broadcastMsg(buf, strlen(buf));
        delete [] buf;
        return RET_OK;
    }

    RET_CODE readAndParse(MessageInfo *pmsg_info);

    RET_CODE readMsg(char * buf, int buf_len);

    void sendMsg(int code, const char* data, int len);
    void sendMsg(int fd, int code, const char* data, int len);

    RET_CODE sendToSquare(const char *buf, int len){
        std::shared_ptr<UserMap> tmp;
        {
            tmp = g_users;
        }
        for(auto iter = tmp->begin(); iter != tmp->end(); ++iter){
            int sock = iter->first;
            if(sock != m_socket){
                printf("send to user %s\n", iter->second.username.c_str());
                sendMsg(sock, CODE_BROADCAST, buf, len);
            }
        }
        return RET_OK;
    }
    
};


class SessionMng{
private:
    SessionMng(){}
    ~SessionMng(){}
public:
    static SessionMng* getInstance(){
        static SessionMng m_session_mng;
        return &m_session_mng;
    }

    bool addSession(int fd, std::shared_ptr<Session> sess){
        if(m_session_mp.find(fd) != m_session_mp.end()){
            return false;
        }
        m_session_mp[fd] = sess;

        return true;
    }

    bool delSession(int fd){
        auto iter = m_session_mp.find(fd);
        if(iter != m_session_mp.end()){
            char goodby[100];
            memset(goodby, 0, sizeof(goodby));
            std::shared_ptr<UserMap> tmp;
            {
                tmp = g_users;
            }
            std::string username = tmp->find(fd)->second.username;
            snprintf(goodby, 100, "%s 离开聊天室.\n", username.c_str());
            iter->second->broadcastMsg(goodby, strlen(goodby));
            m_session_mp.erase(iter);
        }
        return true;
    }

    bool handleSession(int fd){
        auto iter = m_session_mp.find(fd);
        if(iter == m_session_mp.end()){
            return false;
        }
        //处理
        auto sess = iter->second;
        if(sess->handleMsg() == RET_EXIT){
            delSession(fd);
        }
        return true;
    }

    auto begin(){
        return m_session_mp.begin();
    }
    auto end(){
        return m_session_mp.end();
    }

private:
    std::unordered_map<int, std::shared_ptr<Session>> m_session_mp;
};

// SessionMng* SessionMng::getInstance(){
//     static SessionMng m_session_mng;
//     return &m_session_mng;
// }

#endif