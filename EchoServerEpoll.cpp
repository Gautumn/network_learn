#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <fcntl.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>



#define PORT_NUM 7777
#define MAX_BUFF 1024

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

char buff[MAX_BUFF + 1];


int setNonBlocking(int fd) {
    int oldOpt = fcntl(fd, F_GETFL);
    int newOpt = oldOpt | O_NONBLOCK;
    fcntl(fd, F_SETFL, newOpt);
    return oldOpt;
}

void addFdIn(int epollfd, int fd, bool enable_et) {
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    if (enable_et) {
        event.events |= EPOLLET;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setNonBlocking(fd);
}

void ModifyFdOut(int epollfd, int fd, bool enable_et) {
    epoll_event event;
    event.events = EPOLLOUT;
    event.data.fd = fd;
    if (enable_et) {
        event.events |= EPOLLET;
    }
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
    setNonBlocking(fd);
}

void ModifyFdIn(int epollfd, int fd, bool enable_et) {
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    if (enable_et) {
        event.events |= EPOLLET;
    }
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
    setNonBlocking(fd);
}


void lt(int server_fd, int epollfd, epoll_event *events, int eventsCnt) {

    for (int i = 0; i < eventsCnt; i++) {
        int fd = events[i].data.fd;
        if (fd == server_fd) {
            /// accept
            struct sockaddr_in server_addr, client_addr;
            socklen_t len = sizeof(struct sockaddr);
            int newfd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
            /// using lt mode
            addFdIn(epollfd, newfd, false);
            printf("get connection from ip:%s, port:%d\n",
                    inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }
        else if (events[i].events & EPOLLIN) {
            memset(buff, 0, sizeof(buff));
            int len = recv(fd, buff, MAX_BUFF - 1, 0);
            if (len > 0) {
                // ASCII-10: 换行, ASCII-13: 回车
                printf("> Receive %d Bytes: %s \n", (int)strlen(buff), buff);
            } else if (len == 0) {
                printf("client quit ...\n");
                break;
            }
            /// change to out
            ModifyFdOut(epollfd, fd, false);
        } else if (events[i].events & EPOLLOUT) {
            /// send data.
            int len = send(fd, buff, strlen(buff), 0);
            if (len > 0) {
                printf("> Send %d Bytes: %s\n", (int)strlen(buff), buff);
            } else if (len ==0) {
                printf("client quit...\n");
                break;
            }
            /// change to in
            ModifyFdIn(epollfd, fd, false);
        }
    }
}

int main() {
    int server_fd;

    struct sockaddr_in server_addr, client_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_port = htons(PORT_NUM);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        handle_error("bind");
    }

    listen(server_fd, 5);

    epoll_event events[5];
    int epollfd = epoll_create(5);
    addFdIn(epollfd, server_fd, true);

    while (1) {
        int evetsCnt = epoll_wait(epollfd, events, 5, -1);

        /// using Level Trigger(LT) 模式
        lt(server_fd, epollfd, events, evetsCnt);
    }
    close(server_fd);
}