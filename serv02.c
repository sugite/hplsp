#include"unp.h"

static int nchildren;
static pid_t *pids;
long  *cptr, *meter(int);

void sig_int(int sig)
{
	int i ;
	void pr_cpu_time(void);

	for ( i = 0; i < nchildren; ++i)
	{
		/* code */
		kill(pids[i], SIGTERM);
	}
	while(wait(NULL)>0);

	if(errno != ECHILD)
		err_sys("wait error");

	pr_cpu_time();

	for( i = 0 ; i < nchildren ; ++i){
		printf("%ld\n",*(cptr+i));
	}

	exit(0);
}

int main(int argc, char *argv[])
{
	int listenfd , i;
	socklen_t addrlen;
	void sig_int(int);
	pid_t child_make(int,int,int);

	if(argc == 3)
		listenfd = Tcp_listen(NULL, argv[1], &addrlen);
	else if(argc == 4)
		listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
	else
		err_quit("usage: serv02 [<host>] <port#> <#children>");

	nchildren = atoi(argv[argc-1]);
	pids = (pid_t*)Calloc(nchildren, sizeof(pid_t));

	
	cptr = meter(nchildren);

	for (i = 0; i < nchildren; ++i)
	{
		/* code */
		pids[i] = child_make(i, listenfd, addrlen);
	}

	Signal(SIGINT, sig_int);
	for(;;)
		pause();
}