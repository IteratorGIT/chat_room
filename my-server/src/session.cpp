#include "session.h"
#include <string.h>
#include <unistd.h>

std::shared_ptr<UserMap> g_users = std::make_shared<UserMap>();
Lock g_mtx;

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

RET_CODE Session::readMsg(char * buf, int buf_len){
    memset(buf,0, buf_len);
    //循环读取，直到读完
    //todo:对于可能出现的读不完的情况呢？？？
    int curlen = 0;
    while(curlen < buf_len){
        int len = read(m_socket, buf+curlen, buf_len-curlen);
        if(len < 0){
            //读完了
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                break;
            }
            return RET_ERROR;
        }else if(len == 0){
            return RET_EXIT;
        }else{
            curlen += len;
        }
    }
    if(curlen == buf_len){
        return RET_OK;
    }
    return RET_ERROR;
}

void Session::sendMsg(int fd, int code, const char* data, int len){
    char *buf = new char[sizeof(MessageHeader) + len];

    MessageHeader * pheader = (MessageHeader*)buf;
    pheader->magic = MAGIC;
    pheader->code = code;
    pheader->length = len;
    char *body = buf + sizeof(MessageHeader);
    if(len > 0){
        memcpy(body, data, len);
    }
    write(fd, buf, sizeof(MessageHeader) + len);
    delete buf;
}
void Session::sendMsg(int code, const char* data, int len){
    sendMsg(m_socket, code, data, len);
}