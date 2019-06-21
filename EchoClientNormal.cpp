#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>

#include <resolv.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


#define MAX_BUFF 1024

int main(void) {
    int sockfd, len;

    struct sockaddr_in dest;
    char cmdLineBuff[MAX_BUFF];;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(7777);
    dest.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sockfd, (struct sockaddr *)&dest, sizeof(dest));

    while(1) {
        /// get data from command line
        memset(cmdLineBuff, 0, sizeof(cmdLineBuff));
        fgets(cmdLineBuff, sizeof(cmdLineBuff), stdin);
        /// send data to server
        len = send(sockfd, cmdLineBuff, strlen(cmdLineBuff), 0);
        if (len < 0) {
            printf("> Send message error");
        } else {
            printf("> Send message %d Bytes: %s", len, cmdLineBuff);
        }
        /// recevie data from server
        memset(cmdLineBuff, 0, sizeof(cmdLineBuff));
        len = recv(sockfd, cmdLineBuff, sizeof(cmdLineBuff), 0);
        if (len < 0) {
            printf("> Receive error");
        } else if (len > 0) {
            printf("> Receive message %d Bytes: %s", len, cmdLineBuff);
        }
    }

    close(sockfd);
    return 0;
}