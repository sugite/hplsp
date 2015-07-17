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
#define PORT 12357
#define MAXLINE 1024
#define LISTENQ 5
#define OPEN_MAX 1000
#define INFTIM -1

static int socket_bind(const char *ip, int port);
static void do_poll(int listenfd);
static void handle_connection(struct pollfd *connfds, int num);

int main(int argc, char* argv[])
{
	int listenfd, connfd, sockfd;
	listenfd = socket_bind(IP, PORT);
	int ret = listen(listenfd, LISTENQ);
	assert(ret != -1);
	do_poll(listenfd);
	return 0;
}

static int socket_bind(const char* ip, int port)
{
	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);

	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);
	int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
	assert(ret != -1);
	return listenfd ;
}

static void do_poll(int listenfd)
{
	int sockfd, connfd ;
	struct sockaddr_in clientaddr;
	socklen_t cli_addr_len = sizeof(clientaddr);
	struct pollfd clientfds[OPEN_MAX];
	int maxi ;
	int i ;
	int nready ;
	clientfds[0].fd = listenfd;
	clientfds[0].events = POLLIN;
	for(i = 1 ; i < OPEN_MAX; ++i){
		clientfds[i].fd = -1;
	}

	maxi = 0;
	for(;;){
		nready = poll(clientfds, maxi + 1, INFTIM);
		assert(nready != -1);
		if(clientfds[0].revents & POLLIN){
			connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &cli_addr_len);
			if(connfd == -1 && errno == EINTR) continue;
			else if(connfd == -1){
				perror("accept");
				exit(1);
			}
			fprintf(stdout, "accept a new client: %s:%d\n", inet_ntoa(clientaddr.sin_addr),clientaddr.sin_port);
			for(i = 1 ; i < OPEN_MAX ; ++i){
				if(clientfds[i].fd == -1){
					clientfds[i].fd = connfd;
					break;
				}
			}
			if(i == OPEN_MAX){
				fprintf(stderr, "too many clients");
				exit(1);
			}
			clientfds[i].events = POLLIN;
			maxi = (i > maxi ? i : maxi );
			if(--nready<=0) continue;
		}
		handle_connection(clientfds, maxi);
	}
}

static void handle_connection(struct pollfd* clientfds, int num)
{
	int i ,n ;
	char buf[MAXLINE];
	memset(buf,'\0',MAXLINE);
	for(i = 1 ; i <= num ; ++i){
		if(clientfds[i].fd < 0) continue;
		if(clientfds[i].revents & POLLIN){
			n = read(clientfds[i].fd, buf, MAXLINE-1);
			if(n==0){
				close(clientfds[i].fd);
				clientfds[i].fd = -1;
				continue;
			}
			write(STDOUT_FILENO, buf, n);
			write(clientfds[i].fd, buf, n);
		}
	}
}
