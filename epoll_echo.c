#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<assert.h>
#include<unistd.h>

#include<netinet/in.h>
#include<sys/socket.h>
#include<poll.h>
#include<sys/epoll.h>
#include<arpa/inet.h>
#include<sys/types.h>

#define IP "127.0.0.1"
#define PORT 8787
#define MAXLINE 1024
#define LISTENQ 5
#define FDSIZE 1000
#define EPOLLEVENTS 100


static int socket_bind(const char* ip, int port);
static void do_epoll(int listenfd);
static void handle_events(int epollfd, struct epoll_event *events, int num, int listenfd, char* buf);
static void handle_accept(int epollfd, int listenfd);
static void do_read(int epollfd, int fd, char* buf);
static void do_write(int epollfd, int fd, char* buf);
static void modify_event(int epollfd, int fd, int state);
static void add_event(int epollfd, int fd, int state);
static void delete_event(int epollfd, int fd, int state);

int main(void)
{
	int listenfd;
	listenfd = socket_bind(IP, PORT);
	assert(listen(listenfd, LISTENQ)!=-1);
	do_epoll(listenfd);
	return 0;
}

int socket_bind(const char* ip, int port)
{
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);
	struct sockaddr_in server_address;
	bzero(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	inet_pton(AF_INET, ip, &server_address.sin_addr);
	
	int ret = bind(listenfd, (struct sockaddr*)&server_address, sizeof(server_address));
	assert(ret != -1);

	return listenfd;
}

static void do_epoll(int listenfd)
{
	int epollfd;
	struct epoll_event events[EPOLLEVENTS];
	int ret ;
	char buf[MAXLINE];
	memset(buf, '\0', MAXLINE);
	epollfd = epoll_create(FDSIZE);
	add_event(epollfd, listenfd, EPOLLIN);
	for(;;){
		ret = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
		handle_events(epollfd, events, ret, listenfd, buf);
	}
	close(epollfd);
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

static void handle_events(int epollfd, struct epoll_event *events, int num , int listenfd, char*buf)
{
	int i , fd ;
	for(i = 0 ; i < num ; ++i){
		fd = events[i].data.fd ;
		if(fd == listenfd && (events[i].events & EPOLLIN)){
			handle_accept(epollfd, listenfd);
		}else if(events[i].events & EPOLLIN)
			do_read(epollfd, fd, buf);
		else if(events[i].events & EPOLLOUT)
			do_write(epollfd, fd, buf);
	}
}

static void handle_accept(int epollfd, int listenfd)
{
	int clifd ;
	struct sockaddr_in client_address;
	socklen_t client_address_len = sizeof(client_address);
	clifd = accept(listenfd, (struct sockaddr*)&client_address, &client_address_len);
	assert(clifd != -1);
	printf("accept a new client : %s:%d\n",inet_ntoa(client_address.sin_addr),client_address.sin_port);
	add_event(epollfd, clifd, EPOLLIN);
}

static void do_read(int epollfd, int fd, char* buf)
{
	int nread = read(fd, buf, MAXLINE);
	if(nread == -1){
		perror("read");
		close(fd);
		delete_event(epollfd, fd, EPOLLIN);
	}else if (nread == 0){
		fprintf(stderr, "client close. \n");
		close(fd);
		delete_event(epollfd, fd, EPOLLIN);
	}else{
		printf("read message is : %s",buf);
		modify_event(epollfd, fd, EPOLLIN);
	}
}

static void do_write(int epollfd, int fd, char* buf)
{
	int nwrite = write(fd, buf, strlen(buf));
	if(nwrite == -1){
		perror("write");
		close(fd);
		delete_event(epollfd, fd, EPOLLOUT);
	}else{
		modify_event(epollfd, fd, EPOLLOUT);
	}
	memset(buf, '\0', MAXLINE);
}
