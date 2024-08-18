#ifndef SRC_COMMON_H
#define SRC_COMMON_H
#include <string>
//不要包含其他自定义头文件
enum RET_CODE{
    RET_OK,
    RET_ERROR,
    RET_EXIT, //客户端退出
};

//最高支持30字节
struct LoginParam{
    int username_len;
    char username[30];//账号
    int password_len;
    char password[30];//密码
    //nickname 昵称
};
#endif