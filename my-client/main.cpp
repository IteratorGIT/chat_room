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

#include "session.h"

#define MAX_EVENTS_NUM 2
#define MAX_MSG_SIZE 512
#define BUF_SIZE 1024

int setnonblocking( int fd )
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}

void addfd( int epollfd, int fd, bool enable_et = true)
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
    // std::string ip = "127.0.0.1";
    std::string ip = "192.168.28.131";
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

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    
    int ret = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret <0){
        printf("connecting faill.\n");
        return 0;
    }

    int epfd = epoll_create(5);
    if(epfd < 0){
        perror("epoll create error:");
        return 0;
    }

    Session sess(sock);
    sess.showMainMenu();

    addfd(epfd, sock);
    addfd(epfd, STDIN_FILENO);
    epoll_event events[MAX_EVENTS_NUM];

    int pipe_fd[2];
    pipe(pipe_fd);

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
            if(fd == STDIN_FILENO){
                sess.handleInput();

            }else if(fd == sock){
                //普通数据
                //来自服务器的广播
                sess.handleMsg();
            }
        }

    }
    close(sock);
    close(pipe_fd[1]);
    close(pipe_fd[0]);

    return 0;
}