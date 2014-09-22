/*************************************************************************
    > File Name: client.cc
    > Author: dcant
    > Mail: zangzhida@gmail.com 
    > Created Time: Wed 18 Dec 2013 09:05:23 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>
#define MAXBUF 1024
#define handle_error(msg) \
	do {\
		perror(msg);\
		exit(EXIT_FAILURE);\
	}while(0)


int main(int argc, char ** argv)
{
	int sockfd;
	socklen_t len;
	struct sockaddr_in serveraddr;
	int ret;
	int maxfd;
	struct timeval tv;
	fd_set fs;
	char buf[MAXBUF + 1];
	if(argc < 2)
	{
		printf("Usage: %s serverip serverport\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	bzero(&serveraddr, sizeof(serveraddr));
	if(argv[1])
		serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	if(argv[2])
		serveraddr.sin_port = htons(atoi(argv[2]));
	serveraddr.sin_family = AF_INET;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		handle_error("create socket error!");
	len = sizeof(serveraddr);
	if((ret = connect(sockfd, (struct sockaddr*)&serveraddr, len)) == -1)
		handle_error("connect error!");
	printf("\033[1;31m***Connected to server***\033[0m\n");
	maxfd = sockfd;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	bzero(buf, MAXBUF + 1);
	while(true)
	{
		FD_ZERO(&fs);
		FD_SET(0, &fs);
		FD_SET(sockfd, &fs);
		int ret = select(maxfd + 1, &fs, NULL, NULL, &tv);
		if(ret == -1)
			handle_error("select error!");
		else if(ret == 0)
			continue;
		else {
			if(FD_ISSET(0, &fs))
			{
 				bzero(buf, MAXBUF +  1);
				fgets(buf, MAXBUF, stdin);
				if(!strncasecmp(buf, "quit", 4))
				{
 					printf("\033[1;34mWill quit now!\033[0m\n");
					break; 
				}
				len = send(sockfd, buf, strlen(buf) - 1, 0);
				if(len == 0)
					printf("\tsend error!\n");
			}
			if(FD_ISSET(sockfd, &fs))
			{
				bzero(buf, MAXBUF + 1); 
				len = recv(sockfd, buf, MAXBUF, 0);
				if(len > 0){
					printf("\033[1;32mReceived: \033[0m");
					fputs(buf, stdout);
					fputs("\n", stdout);
				}
				else if(len < 0){
					printf("\trecv error!\n");
				}else{
					printf("\033[1;34mServer closed!\033[0m\n");
					break;
				}
			}
		}
	}
	close(sockfd);
	return 0;
}
