#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>

#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<sys/types.h>
#include<assert.h>
#define IPADDRESS	"192.168.159.128"
#define PORT		11256
#define MAXLINE		1024
#define LISTENQ		5

static int socket_bind(const char* ip, int port);
static void do_select(int listenfd);
static void handle_connection(int *connfds,int num, fd_set  *prset, fd_set *pallset);

int main(int argc, char* argv[])
{
	int listenfd, connfd, sockfd;
	struct sockaddr_in cliaddr;
	socklen_t cliaddrlen;
	listenfd = socket_bind(IPADDRESS, PORT);
	listen(listenfd, LISTENQ);
	do_select(listenfd);
	return 0;
}

static int socket_bind(const char* ip, int port)
{
	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	inet_pton(AF_INET, ip, &address.sin_addr);

	int socketfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(socketfd >= 0);
	int ret = bind(socketfd, (struct sockaddr*)&address, sizeof(address));
	assert(ret != -1);

	return socketfd ;
}

static void do_select(int listenfd)
{
	int connfd, sockfd;
	struct sockaddr_in cliaddr;
	socklen_t cliaddrlen;
	fd_set rset,allset;
	int maxfd, maxi;
	int i;
	int clientfds[FD_SETSIZE];
	int nready ;
	for(i = 0 ; i < FD_SETSIZE ; ++i)
		clientfds[i] = -1;
	maxi = -1;
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	maxfd = listenfd;
	for(;;){
		rset = allset;
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);
		if(nready == -1){
			perror("select:");
			exit(1);
		}
		if(FD_ISSET(listenfd, &rset)){
			connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &cliaddrlen);
			if(connfd == -1){
				if(errno == EINTR) continue;
				else{
					perror("accept:");
					exit(1);
				}
			}
			fprintf(stdout,"accept a new client: %s:%d\n",inet_ntoa(cliaddr.sin_addr),cliaddr.sin_port);
			for(i = 0 ; i < FD_SETSIZE ; ++i){
				if(clientfds[i] < 0){
					clientfds[i] = connfd;
					break;
				}
			}
			if(i == FD_SETSIZE){
				fprintf(stderr,"too many clients.\n");
				exit(1);
			}
			FD_SET(connfd,&allset);
			maxfd = (connfd > maxfd ? connfd : maxfd);
			maxi = (i > maxi ? i : maxi);
			if(--nready <= 0)
				continue;
		}
		handle_connection(clientfds, maxi, &rset, &allset);
	}
}

static void handle_connection(int *connfds, int num, fd_set *prset, fd_set *pallset)
{
	int i,n;
	char buf[MAXLINE];
//	memset(buf,'\0',MAXLINE);
	for(i = 0 ; i <= num ; ++i){
		memset(buf,'\0',MAXLINE);
		if(connfds[i] < 0) continue;
		if(FD_ISSET(connfds[i], prset)){
			n = read(connfds[i], buf, MAXLINE-1);
			if(n==0){
				close(connfds[i]);
				FD_CLR(connfds[i], pallset);
				connfds[i] = -1;
				continue;
			}
			printf("read msg id : %s\n",buf);
//			write(STDOUT_FILENO, buf, n);
			write(connfds[i],buf,n);
		}
	}
}
