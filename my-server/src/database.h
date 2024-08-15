#ifndef DATABASE_H
#define DATABASE_H

#include "common.h"
#include <string>

class DataBase{
private:
    DataBase(){
    }
    ~DataBase(){}
public:
    static DataBase* getInstance(){
        static DataBase db;
        return &db;
    }
    bool init(std::string ip, int port){
        //假装连接
        return true;
    }
    bool login(LoginParam *param){
        //假装登录
        return true;
    }

};

#endif