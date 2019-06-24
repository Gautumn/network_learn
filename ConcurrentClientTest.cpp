#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>

#include <resolv.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_BUFF 1024
#define testCount 300
static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;

void *handleFunc(void *fd) {
    int sockfd = *(int *)fd;

    pthread_mutex_lock(&mut);
    printf("Thread wait sockfd:%d\n", sockfd);
    pthread_cond_wait(&cond, &mut);
    pthread_mutex_unlock(&mut);

    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(7777);
    dest.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret = connect(sockfd, (struct sockaddr *)&dest, sizeof(dest));
    if (ret == -1) {
        printf("Connect failed\n");
    }
    return fd;
}

int main(void) {
    int sockfd[testCount];
    pthread_t tids[testCount];
    for (int i = 0; i < testCount; i++) {
        sockfd[i] = socket(AF_INET, SOCK_STREAM,0);
        pthread_create(&tids[i], NULL, handleFunc, &sockfd[i]);
    }

    while (1) {
        sleep(2);
        pthread_cond_broadcast(&cond);
    }
    return 0;
}