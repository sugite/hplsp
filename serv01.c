#include"unp.h"
#include"apue.h"

void sig_chld(int signo)
{
	pid_t pid ;
	int stat;
	while((pid = waitpid(-1, &stat, WNOHANG)) > 0);
	return ;
}

void sig_int(int signo)
{
	void pr_cpu_time(void);
	pr_cpu_time();
	exit(0);
}

int main(int argc, char* argv[])
{
	int listenfd, connfd;
	pid_t childpid;
	void sig_chld(int), sig_int(int), web_child(int);
	socklen_t clilen,addrlen;
	struct sockaddr* cliaddr;
	if(argc ==2)
		listenfd = Tcp_listen(NULL, argv[1], &addrlen);
	else if(argc == 3)
		listenfd = Tcp_listen(argv[1],argv[2],&addrlen);
	else
		err_quit("usage: serv01 [<host>] <port#>");

	cliaddr = (struct sockaddr*)Malloc(addrlen);

	Signal(SIGCHLD, sig_chld);
	Signal(SIGINT, sig_int);

	for(;;){
		clilen = addrlen;
		if((connfd = accept(listenfd, cliaddr, &clilen)) < 0){
			if(errno == EINTR)
				continue;
			else
				err_sys("accept error");
		}

		if((childpid = Fork())  == 0){
			Close(listenfd);
			web_child(connfd);
			exit(0);
		}
		Close(connfd);
	}
}
