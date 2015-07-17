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
#include<sys/epoll.h>
#include<arpa/inet.h>
#define IP "127.0.0.1"
#define PORT 8787
#define MAXLINE 1024
#define LISTENQ 5
#define FDSIZE 1024
#define EPOLLEVENTS 20

static void handle_connection(int a);
static void handle_events(int a,struct epoll_event* b, int c,int d,char* r);
static void do_read(int a,int b,int c,char* d);
static void do_write(int a,int b,int c,char* d);
static void add_event(int a,int b,int c);
static void delete_event(int a,int b,int c);
static void modify_event(int a,int b,int c);

int main(void)
{
	int sockfd;
	struct sockaddr_in server_address;
	bzero(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	inet_pton(AF_INET, IP, &server_address.sin_addr);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(sockfd>=0);
	int connfd = connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address));
	assert(connfd!=-1);
	handle_connection(sockfd);
	close(sockfd);
	return 0;
}

static void handle_connection(int sockfd)
{
	int epollfd ;
	struct epoll_event events[EPOLLEVENTS];
	char buf[MAXLINE];
	int ret ;
	epollfd = epoll_create(FDSIZE);
	struct epoll_event ev;
	ev.data.fd = STDIN_FILENO;
	ev.events = EPOLLIN;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);
	for(;;){
		ret = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
		handle_events(epollfd, events, ret, sockfd, buf);
	}
	close(epollfd);
}

static void handle_events(int epollfd, struct epoll_event *events, int num, int sockfd, char*buf)
{
	int fd,i;
	for(i = 0 ; i< num ; ++i)
	{
		fd = events[i].data.fd;
		if(events[i].events & EPOLLIN){
			do_read(epollfd, fd, sockfd, buf);
		}else if(events[i].events & EPOLLOUT){
			do_write(epollfd, fd, sockfd, buf);
		}
	}
}

static void do_read(int epollfd, int fd, int sockfd, char*buf)
{
	int nread = read(fd, buf, MAXLINE);
	if(nread == -1){
		perror("read");
		close(fd);
	}else if(nread == 0){
		fprintf(stderr,"server close\n");
		close(fd);
	}else{
		if(fd == STDIN_FILENO)
			add_event(epollfd, sockfd, EPOLLOUT);
		else{
			delete_event(epollfd, sockfd, EPOLLIN);
			add_event(epollfd, STDOUT_FILENO, EPOLLOUT);
		}
	}
}

static void do_write(int epollfd, int fd, int sockfd, char* buf)
{
	int nwrite = write(fd, buf, strlen(buf));
	if(nwrite == -1){
		perror("write");
		close(fd);
	}else{
		if(fd == STDOUT_FILENO)
			delete_event(epollfd, fd, EPOLLOUT);
		else
			modify_event(epollfd, fd, EPOLLIN);
	}

	memset(buf, '\0', MAXLINE);
}


static void add_event(int epollfd, int fd, int state)
{
		struct epoll_event ev;
			ev.events = state;
				ev.data.fd = fd;
					epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

static void modify_event(int epollfd, int fd, int state)
{
		struct epoll_event ev;
			ev.events = state;
				ev.data.fd = fd;
					epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}

static void delete_event(int epollfd, int fd, int state)
{
		struct epoll_event ev ;
			ev.events = state;
				ev.data.fd = fd ;
					epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

