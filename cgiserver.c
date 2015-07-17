#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>

#define BUFFER_SIZE 512

int main(int argc, char* argv[])
{
	if(argc <= 2){
		printf("usage : %s ip_address port_number \n",basename(argv[0]));
		return 1;
	}

	const char* ip = argv[1];
	int port = atoi(argv[2]);

	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_family = AF_INET;
	address.sin_port = htons(port);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(sockfd >= 0);

	int ret = bind(sockfd, (struct sockaddr*)&address, sizeof(address));
	assert(ret != -1);

	ret = listen(sockfd, 5);
	assert(ret != -1);

	struct sockaddr_in client ;
	socklen_t client_addrlength = sizeof(client);
	int connfd = accept(sockfd, (struct sockaddr*)&client, (socklen_t*)&client_addrlength);
	if(connfd < 0)
		printf("errno is %d\n",errno);
	else{
		close(STDOUT_FILENO);
		dup(connfd);
		printf("abcd\n");
		close(connfd);
	}

	close(sockfd);
	return 0;
}
