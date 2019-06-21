#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>



#define PORT_NUM 7777
#define MAX_BUFF 1024

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)


void echo(int fd) {
    int len;
    char buff[MAX_BUFF + 1];
    memset(buff, 0, sizeof(buff));
    do {
        len = recv(fd, buff, MAX_BUFF, 0);
        if (len > 0) {
            // ASCII-10: 换行, ASCII-13: 回车
            printf("> Receive %d Bytes: %s \n", len, buff);
        } else if (len == 0) {
            printf("client quit ...\n");
            break;
        }

        len = send(fd, buff, strlen(buff), 0);
        if (len > 0) {
            printf("> Send %d Bytes: %s\n", len, buff);
        } else if (len ==0) {
            printf("client quit...\n");
            break;
        }
    } while (1);

    close(fd);
}

int main() {
    pid_t pid;
    int server_fd, conn_fd;

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

    while(1) {
        socklen_t len = sizeof(struct sockaddr);

        conn_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);

        if(conn_fd > 0) {
            printf("get connection from ip:%s, port:%d\n",
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }

        if((pid = fork()) == 0) {
            /// Child process
            echo(conn_fd);
        }
    }

    close(server_fd);
}