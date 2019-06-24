#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>

#define PORT_NUM 7777
#define MAX_BUFF 1024

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

static int server_fd;


// Ctrl+C could not stop the accept function
void handleSigInt(int signum) {
    printf("Get force stop signal\n");
}

void *echo(void *fd) {
    int len;
    char buff[MAX_BUFF + 1];
    int sockfd = *(int *)fd;

    do {
        memset(buff, 0, sizeof(buff));
        len = recv(sockfd, buff, MAX_BUFF, 0);
        if (len > 0) {
            // ASCII-10: 换行, ASCII-13: 回车
            printf("> Receive %d Bytes Contents: %s \n", len, buff);
        } else if (len == 0) {
            printf("client quit ...\n");
            break;
        }

        len = send(sockfd, buff, strlen(buff), 0);
        if (len > 0) {
            printf("> Send %d Bytes Contents: %s\n", len, buff);
        } else if (len ==0) {
            printf("client quit...\n");
            break;
        }
    } while (1);

    close(sockfd);

    return fd;
}

int main() {
    pid_t pid;
    int conn_fd;
    pthread_t echoId;

    struct sockaddr_in server_addr, client_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_port = htons(PORT_NUM);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        handle_error("bind");
    }

    signal(SIGINT, handleSigInt);

    listen(server_fd, 128);

    while(1) {
        socklen_t len = sizeof(struct sockaddr);

        conn_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);

        if(conn_fd > 0) {
            printf("get connection from ip:%s, port:%d\n",
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }

        /// Create new thread
        pthread_create(&echoId, NULL, echo, &conn_fd);
    }

    close(server_fd);
}