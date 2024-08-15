#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <string>
#include <unordered_map>
#include <memory>

#include "src/session.h"

std::unordered_map<int, std::shared_ptr<User>> mp_users;

#define MAX_EVENTS_NUM 1024
#define MAX_MSG_SIZE 512
#define BUF_SIZE 1024

int setnonblocking( int fd )
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}

void addfd( int epollfd, int fd, bool enable_et = false)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if( enable_et )
    {
        event.events |= EPOLLET;
    }
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
    setnonblocking( fd );
}


int main(int argc, char *argv[]){
    std::string ip = "0.0.0.0";
    int port = 8888;
    if(argc >= 3){
        ip = std::string(argv[1]);
        port = atoi(argv[2]);
    }
    printf("using ip: %s, port: %d\n", ip.c_str(), port);

    //服务器地址
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);
    server_addr.sin_port = htons(port);

    //套接字
    int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        perror("Socket Error:");
        return 0;
    }
    //socket重用，方便快速重启
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&listen_fd, sizeof(listen_fd));

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Bind error:");
        return 0;
    }

    if (listen(listen_fd, 4) == -1)
    {
        perror("Listen error:");
        return 0;
    }

    printf("listening...\n");

    int epfd = epoll_create(5);
    if(epfd < 0){
        perror("epoll create error:");
        return 0;
    }
    addfd(epfd, listen_fd);
    epoll_event events[MAX_EVENTS_NUM];
    
    //事件循环
    bool stop = false;
    while(!stop){
        int ret = epoll_wait(epfd, events, MAX_EVENTS_NUM, -1);
        if(ret < 0){
            printf("epoll failure.\n");
            break;
        }
        for(int i=0;i<ret;i++){
            int fd = events[i].data.fd;
            if(fd == listen_fd){
                //新连接
                //将新连接加入连接map
                struct sockaddr_in client_addr;
                socklen_t client_addr_len;
                int connfd = accept( listen_fd, ( struct sockaddr* )&client_addr, &client_addr_len );
                addfd(epfd, connfd);


                //记录用户信息
                std::string username = "游客";
                username += std::to_string(connfd);
                mp_users[connfd] = std::make_shared<User>(username,connfd);
                //加入epoll
                addfd(epfd, connfd);
                printf("new client: %d\n", connfd);

                //进入聊天室欢迎语
                char welcome[100];
                memset(welcome, 0, sizeof(welcome));
                snprintf(welcome, 100, "%s 进入聊天室.\n", username.c_str());
                for(auto it = mp_users.begin(); it != mp_users.end(); ++it){
                    int fd = it->first;
                    if(fd != connfd){
                        send(fd, welcome, strlen(welcome), 0);    
                    }
                }

            }else if(events[i].events & EPOLLIN){
                //普通数据
                //读取数据，并进行处理
                int connfd = events[i].data.fd;
                char msg[MAX_MSG_SIZE];
                memset(msg,0,MAX_MSG_SIZE);
                ret = recv(connfd, msg, MAX_MSG_SIZE-1, 0);
                if(ret < 0){
                    printf("接受数据错误\n");
                    continue;
                }else if(ret == 0){

                    char leave[100];
                    memset(leave, 0, sizeof(leave));
                    snprintf(leave, 100, "%s 离开聊天室.\n", mp_users[connfd]->username.c_str());
                    for(auto it = mp_users.begin(); it != mp_users.end(); ++it){
                        int fd = it->first;
                        if(fd != connfd){
                            send(fd, leave, strlen(leave), 0);    
                        }
                    }
                    //关闭连接
                    close(connfd);
                    //删除用户数据
                    mp_users.erase(connfd);
                    printf("%d closed, %d alive\n", connfd, (int)mp_users.size());
                }else{
                    //将消息广播
                    char buf[BUF_SIZE];
                    memset(buf, 0, BUF_SIZE);
                    auto pUser = mp_users[connfd];
                    snprintf(buf, BUF_SIZE-1, ">> %s: %s", pUser->username.c_str(), msg);
                    for(auto it = mp_users.begin(); it != mp_users.end(); ++it){
                        int fd = it->first;
                        if(fd != connfd){
                            send(fd, buf, strlen(buf), 0);    
                        }
                    }
                }
            }
        }

    }
    



    return 0;
}