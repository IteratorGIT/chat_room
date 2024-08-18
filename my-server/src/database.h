#ifndef DATABASE_H
#define DATABASE_H

#include "log.h"
#include "common.h"
#include <mysql/mysql.h>
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
    bool init(std::string host, int port, 
        std::string user, std::string password, std::string database){
        MYSQL *con = NULL;
		con = mysql_init(con);

		if (con == NULL)
		{
			Log(mysql_error(con));
			exit(1);
		}
		con = mysql_real_connect(con, 
                host.c_str(), user.c_str(), password.c_str(), 
                database.c_str(), port, NULL, 0);

		if (con == NULL)
		{
			Log(mysql_error(con));
			exit(1);
		}
        m_conn = con;
        m_host = host;
        m_port = port;
        m_user = user;
        m_password = password;
        m_database = database;
        return true;
    }
    bool destroy(){
        mysql_close(m_conn);
    }
    bool login(LoginParam *param){
        std::string username(param->username);
        std::string password(param->password);
        if(check(username) && check(password)){
            return false;
        }
        std::string sql = "";
        sql += "select count(*) from user where username = '";
        sql += std::string(param->username);
        sql += "' and password = '";
        sql += std::string(param->password);
        sql += "';";
        if(mysql_query(m_conn, sql.c_str())){
            LOG_ERROR("select error:%s", mysql_error(m_conn));
            return false;
        }
        MYSQL_RES *result = mysql_store_result(m_conn);
        if(result->row_count != 1){
            return false;
        }  
        return true;
    }
private:
    bool check(std::string str){
        //不得含有sql注入字段
        return true;
    }
private:
    MYSQL * m_conn;

    std::string m_host;
    int m_port;
    std::string m_user;
    std::string m_password;
    std::string m_database;
};

#endif