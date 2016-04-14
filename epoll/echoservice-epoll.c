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
#include "epoll.h"

void dorequest(int fd);
int make_socket_non_blocking(int fd);

extern struct epoll_event *events;

int main()
{
	int listenfd, optval = 1;
	int i;
	int port = 9501;
	struct sockaddr_in serveraddr;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

	bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serveraddr.sin_port = htons((unsigned short)port);

    bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    printf("%d\n", errno);

    listen(listenfd, 1024);
    printf("%d\n", errno);

    make_socket_non_blocking(listenfd);
    printf("%d\n", errno);

    int epfd = zv_epoll_create(0);
    struct epoll_event event;

    printf("start..\n");
    event.data.fd = listenfd;
    event.events = EPOLLIN | EPOLLET;
    zv_epoll_add(epfd, listenfd, &event);

    struct sockaddr_in clientaddr;
    socklen_t inlen = sizeof(clientaddr);      // 一定要赋值
    memset(&clientaddr, 0, sizeof(struct sockaddr_in)); 

    int n, fd, infd;
    while (1) {
        n = zv_epoll_wait(epfd, events, MAXEVENTS, -1);

        for (i = 0; i < n; i++) {
            fd = events[i].data.fd;
            printf("asd\n");
            printf("%d\n", fd);  
            printf("%d\n", listenfd);         
            if (listenfd == fd) {
                while(1) {
                    memset(&clientaddr, 0, sizeof(struct sockaddr_in)); 
                    infd = accept(fd, (struct sockaddr *)&clientaddr, &inlen);
                    printf("%d\n", infd);
                    printf("%d\n", errno); 
                    if (infd == -1) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                            break;
                        } 
                        else {
                            break;
                        }
                    }
                    make_socket_non_blocking(infd);  
                    event.data.fd = infd;              
                    event.events = EPOLLIN | EPOLLET;
                    zv_epoll_add(epfd, infd, &event);
                }
                
            }   // end of while of accept
            else {
                dorequest(fd);
            }
        } 
    }
    close(listenfd);
    return 0;
}

void dorequest(int fd)
{
	char in[1024];
    memset(in, 0, 1024);    //注意清空接收区
	char *out = "hello\n";
	int n = read(fd, in, 1023);
	if(n <= 0) {
		
	}
	else {
		printf("receive: %s", in);
		write(fd, out, 6);
        usleep(10000);
	}
	close(fd);
}

int make_socket_non_blocking(int fd) 
{
    int flags, s;
    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl(fd, F_SETFL, flags);
    if (s == -1) {
        return -1;
    }

    return 0;
}