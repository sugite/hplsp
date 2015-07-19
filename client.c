#include"unp.h"
#include"apue.h"

#define MAXN 16384

int main(int argc, char* argv[])
{
	int i,j,fd,nchildren,nloops,nbytes;

	pid_t pid;
	ssize_t n;
	char request[MAXLINE], reply[MAXN];

	if(argc != 6)
		err_quit("usage: client <hostname or IP > <port> <#children> <loops> <nbytes> ");

	nchildren = atoi(argv[3]);
	nloops = atoi(argv[4]);
	nbytes = atoi(argv[5]);
	snprintf(request, sizeof(request), "%d\n", nbytes);

	for(i = 0 ; i < nchildren ; ++i){
		if((pid = Fork()) == 0){
			for(j = 0 ; j < nloops ; ++j){
				fd = Tcp_connect(argv[1], argv[2]);
				Write(fd, request, strlen(request));
				if((n = Readn(fd, reply, nbytes)) != nbytes)
					err_quit("server returned %d bytes", n);
				Close(fd);
			}
			printf("child %d done\n", i);
			exit(0);
		}
	}

	while(wait(NULL) >0) ;
	if(errno != ECHILD)
		err_sys("wait error");
	exit(0);
}
