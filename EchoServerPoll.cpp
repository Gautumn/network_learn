#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/time.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>



#define PORT_NUM 7777
#define MAX_BUFF 1024

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main() {
    int server_fd;

    struct sockaddr_in server_addr, client_addr;
    char buff[MAX_BUFF + 1];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_port = htons(PORT_NUM);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        handle_error("bind");
    }

    listen(server_fd, 5);

    int fdCount;
    pollfd fds[2];

    fdCount = 1;
    fds[0].fd = server_fd;
    fds[0].events = POLLIN | POLLERR;
    fds[0].revents = 0;

    while (1) {
        poll(fds, fdCount, -1);

        if (fds[0].revents == POLLIN) {
            socklen_t len = sizeof(struct sockaddr);
            int newfd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
            fdCount = 2;
            fds[1].fd = newfd;
            fds[1].events = POLLIN;
            fds[1].revents = 0;
            printf("get connection from ip:%s, port:%d\n",
                    inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }
        if (fds[1].revents == POLLIN) {
            /// receive data.
            memset(buff, 0, sizeof(buff));
            int len = recv(fds[1].fd, buff, MAX_BUFF, 0);
            if (len > 0) {
                // ASCII-10: 换行, ASCII-13: 回车
                printf("> Receive %d Bytes: %s \n", (int)strlen(buff), buff);
            } else if (len == 0) {
                printf("client quit ...\n");
                break;
            }

            // change to send
            fds[1].events = POLLOUT;
        }
        if (fds[1].revents == POLLOUT) {
            /// send data.
            int len = send(fds[1].fd, buff, strlen(buff), 0);
            if (len > 0) {
                printf("> Send %d Bytes: %s\n", (int)strlen(buff), buff);
            } else if (len ==0) {
                printf("client quit...\n");
                break;
            }

            // change to receive
            fds[1].events = POLLIN;
        }
    }


    close(server_fd);
}