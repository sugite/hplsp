#include"unp.h"
#include"child.h"

pid_t child_make(int i, int listenfd, int addrlen)
{
	int		sockfd[2];
	pid_t 	pid;
	void 	child_main(int, int, int);
	Socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd);
	if((pid = Fork()) > 0){
		Close(sockfd[1]);
		cptr[i].child_pid = pid;
		cptr[i].child_pipefd = sockfd[0];
		cptr[i].child_status = 0;
		return pid;
	}

	Dup2(sockfd[1], STDERR_FILENO);
	Close(sockfd[0]);
	Close(sockfd[1]);
	Close(listenfd);
	child_main(i, listenfd, addrlen);
}

void child_main(int i, int listenfd, int addrlen)
{
	char 	c;
	int 	connfd;
	ssize_t n;
	void 	web_child(int);

	printf("child %ld starting \n", (long)getpid());
	for(;;){
		if((n = Read_fd(STDERR_FILENO, &c, 1, &connfd)) == 0)
			err_quit("read_fd return 0");
		if(connfd < 0)
			err_quit("no descriptor from read_fd");
		web_child(connfd);
		Close(connfd);
		Write(STDERR_FILENO, "", 1);
	}
}