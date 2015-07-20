#include"unp.h"
#include"child.h"
#include"unpthread.h"

static int nchildren;

void sig_int(int signo)
{
	void pr_cpu_time(void);
	pr_cpu_time();
	exit(0);
}

int main(int argc, char* argv[])
{
	int 	listenfd, i, navail, maxfd, nsel, connfd, rc;
	void	sig_int(int);
	pid_t	child_make(int,int,int);
	ssize_t n;
	fd_set 	rset, masterset;
	socklen_t addrlen,clilen;
	struct sockaddr* cliaddr;

	listenfd = Tcp_listen(argv[1], argv[2], &addrlen);

	FD_ZERO(&masterset);
	FD_SET(listenfd, &masterset);
	maxfd = listenfd;
	cliaddr = Malloc(addrlen);
	nchildren = atoi(argv[argc - 1]);
	navail = nchildren;
	cptr = Calloc(nchildren, sizeof(Child));
	for (i = 0; i < nchildren; ++i)
	{
		child_make(i, listenfd, addrlen);
		FD_SET(cptr[i].child_pipefd, &masterset);
		maxfd = max(maxfd, cptr[i].child_pipefd);
	}
	Signal(SIGINT, sig_int);
	for(;;){
		rset = masterset ;
		if(navail <= 0)
			FD_CLR(listenfd, &rset);
		nsel = Select(maxfd+1, &rset, NULL, NULL, NULL);
		if(FD_ISSET(listenfd, &rset)){
			clilen = addrlen;
			connfd = Accept(listenfd, cliaddr, &clilen);
			for (i = 0; i < nchildren; ++i)
			{
				if(cptr[i].child_status == 0)
					break;
			}
			if(i == nchildren)
				err_quit("no available children");
			cptr[i].child_status = 1;
			cptr[i].child_count++;
			navail--;

			n = Write_fd(cptr[i].child_pipefd, "", 1, connfd);
			Close(connfd);
			if(--nsel==0) continue;
		}

		for(i = 0; i <nchildren ;++i){
			if(FD_ISSET(cptr[i].child_pipefd, &rset)){
				if((n = Read(cptr[i].child_pipefd, &rc, 1)) == 0){
					err_quit("child %d terminated unexpectd",i);
				}
				cptr[i].child_status = 0;
				navail++;
				if(--nsel ==0)break;
			}
		}
	}
}