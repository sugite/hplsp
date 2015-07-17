#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<assert.h>
#include<unistd.h>

#include<netinet/in.h>
#include<sys/socket.h>
#include<poll.h>
#include<sys/types.h>

#define IP "192.168.159.128"
#define PORT 8787
#define MAXLINE 1024
#define LISTENQ 5
#define OPEN_MAX 1000
#define INFTIM -1

#define max(a,b) (a>b ? a : b)

static void handle_connection(int sockfd);

int main()
{
	int sockfd;
	struct sockaddr_in serveraddr;
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT);
	inet_pton(AF_INET, IP, &serveraddr.sin_addr);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(sockfd >= 0);
	int connfd = connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	assert(connfd != -1);
	handle_connection(sockfd);
}

static void handle_connection(int sockfd)
{
	int i , maxi,n,nready ;
	char sendline[MAXLINE], recvline[MAXLINE];
	struct pollfd connfds[2];
	connfds[0].fd = sockfd;
	connfds[0].events = POLLIN;
	connfds[1].fd = STDIN_FILENO;
	connfds[1].events = POLLIN;
	for(;;){
		nready = poll(connfds, 2, -1);
		printf("nready == %d\n",nready);
		if(connfds[0].revents & POLLIN){
			n = read(connfds[0].fd, recvline, MAXLINE-1);
			if(n==0){
				fprintf(stderr,"server is closed\n");
				close(sockfd);
			}
			write(STDOUT_FILENO, recvline, n);
		}
		if(connfds[1].revents & POLLIN){
			n = read(STDIN_FILENO, sendline, MAXLINE-1);
			if(n==0){
				shutdown(sockfd, SHUT_WR);
				continue;
			}
			write(sockfd, sendline, n);
		}
	}
}
