#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/prctl.h>

void dorequest(int fd);

int main()
{
	int listenfd, optval = 1;
	int i;
	int port = 9502;
	struct sockaddr_in serveraddr;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

	bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serveraddr.sin_port = htons((unsigned short)port);

    bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    //printf("%d\n", errno);

    listen(listenfd, 1024);
    //printf("%d\n", errno);

    //make_socket_non_blocking(listenfd);
    //printf("%d\n", errno);

    printf("start..\n"); 

    pid_t pid[4] = {-1, -1, -1, -1};
    for(i = 0; i < 4; i++) {
        pid[i] = fork();
        if(0 == pid[i] || -1 == pid[i]) 
            break;
    }
    if(0 == pid[0]) {
        dorequest(listenfd);
    }
    if(0 == pid[1]) {
        dorequest(listenfd);
    }
    if(0 == pid[2]) {
        dorequest(listenfd);
    }
    if(0 == pid[3]) {
        dorequest(listenfd);
    }
    if(pid[0] != 0 && pid[1] != 0 && pid[2] != 0 && pid[3] != 0) {
        for(i = 0; i < 4; i++) {
            waitpid(pid[i], NULL, 0);
        }
        close(listenfd);
    }
    return 0;
}

void dorequest(int fd)
{
	char in[1024];
	char *out = "hello\n";
    int infd, n;
    prctl(PR_SET_NAME, "worker", NULL, NULL, NULL);
    struct sockaddr_in clientaddr;
    socklen_t inlen = sizeof(clientaddr);      // 一定要赋值
    memset(&clientaddr, 0, sizeof(struct sockaddr_in));
    while(1) {
        memset(in, 0, 1024);    //注意清空接收区
        infd = accept(fd, (struct sockaddr *)&clientaddr, &inlen);
        if(infd < 0) {
        	continue;
        }
        printf("%d\n", infd);
        printf("%d\n", getpid());
        n = read(infd, in, 1023);
        printf("receive: %s", in);
        write(infd, out, 6);
        usleep(10000);
        close(infd);
    }
    close(fd);
}