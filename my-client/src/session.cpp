#include "session.h"
#include <string.h>
#include <unistd.h>

void Session::showMainMenu(){
    char op;
    printf("            ChatRoom powered by hpq       \n");
    printf("┌─────────────────────────────────────────────┐\n");
    printf("│ 1. 注册                        2. 登录       │\n");
    printf("│ 3. 群聊                        4. 私聊       │\n");
    printf("│ 5. 好友管理                    6. 群聊管理   │\n");
    printf("└────────────────────────────────────────────┘\n");
    printf("请您选择(0-6):");fflush(stdout);
}

RET_CODE Session::login(LoginParam *arg){//登录后状态变为已登录
    LoginParam * plogin = (LoginParam*)arg;
    //发送登录请求
    sendMsg(CODE_LOGIN, (const char*)plogin, sizeof(LoginParam));

    //登录应该是阻塞的？
    MessageInfo *pmsg_info = new MessageInfo();
    RET_CODE ret = readAndParse(pmsg_info);

    if(ret != RET_OK){
        return ret;
    }
    
    if(pmsg_info->header->code != CODE_SUCCESS){
        return RET_ERROR;
    }
    m_isLogin = true;
    return RET_OK;
}

RET_CODE Session::readAndParse(MessageInfo *pmsg_info){
    //1. 头部
    int buf_len = sizeof(MessageHeader);
    char *header_buf = new char[buf_len];
    
    RET_CODE ret;
    if((ret = readMsg(header_buf, buf_len)) != RET_OK){
        return ret;
    }
    
    // MessageInfo *pmsg_info = new MessageInfo();
    MessageHeader * pheader = (MessageHeader*)header_buf;
    pmsg_info->header = pheader;
    if(!pheader->check()){
        return RET_ERROR;
    }

    //2. body
    int body_len = pheader->length;
    if(body_len == 0){
        return RET_OK;
    }
    char *body = new char[body_len];
    //读取body
    if((ret = readMsg(body, body_len)) != RET_OK){ 
        return ret;
    }
    pmsg_info->data = body;
    return RET_OK;
}

RET_CODE Session::readfd(int fd,char * buf, int buf_len){
    memset(buf,0, buf_len);
    //循环读取，直到读完
    // printf("reading.\n");
    int curlen = 0;
    while(curlen < buf_len){
        int len = read(fd, buf+curlen, buf_len-curlen);
        if(len < 0){
            //读完了
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                // printf("read completed.\n");
                break;
            }
            return RET_ERROR;
        }else if(len == 0){
            // printf("len == 0\n");
            return RET_EXIT;
        }else{
            curlen += len;
        }
    }
    if(curlen == buf_len){
        return RET_OK;
    }
    return RET_READ_OK;
}


RET_CODE Session::readMsg(char * buf, int buf_len){
    // printf("massage read\n");
    return readfd(m_socket, buf, buf_len);
}

RET_CODE Session::readInput(char * buf, int buf_len){
    // printf("input read\n");
    return readfd(STDIN_FILENO, buf, buf_len);
}


void Session::sendMsg(int code, const char* data, int len){
    char *buf = new char[sizeof(MessageHeader) + len];

    MessageHeader * pheader = (MessageHeader*)buf;
    pheader->magic = MAGIC;
    pheader->code = code;
    pheader->length = len;
    char *body = buf + sizeof(MessageHeader);
    if(len > 0){
        memcpy(body, data, len);
    }
    write(m_socket, buf, sizeof(MessageHeader) + len);
    delete buf;
    
}

RET_CODE Session::mainMenu(const char * operation){
    int op = atoi(operation);//失败返回0
    switch(op){
        case 1:
        case 2:
        case 3:
            m_state = STAT_SQUARE;//暂时直接进入广场群聊
            break;
        case 4:
        case 5:
        default:
            printf("not support now.\n");
            printf("请您选择(0-6):");
            break;
    }
    return RET_OK;
}