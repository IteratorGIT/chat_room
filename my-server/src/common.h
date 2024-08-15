#ifndef SRC_COMMON_H
#define SRC_COMMON_H
#include <string>
enum RET_CODE{
    RET_OK,
    RET_ERROR,
    RET_EXIT, //客户端退出
};

//最高支持30字节
struct LoginParam{
    int username_len;
    char username[30];
    int password_len;
    char password[30];
};
#endif