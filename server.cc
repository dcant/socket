/*************************************************************************
    > File Name: server.cc
    > Author: dcant
    > Mail: zangzhida@gmail.com 
    > Created Time: Tue 17 Dec 2013 04:51:22 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#define MAXBUF 1024
#define handle_error(msg) \
	do {\
		perror(msg);\
		exit(EXIT_FAILURE);\
	}while(0)

void serve(int fd);

int main(int argc, char ** argv)
{
	signal(SIGCHLD, SIG_IGN); //avoid the child thread becoming zomby process
	int sockfd, new_fd;
	socklen_t len;
	struct sockaddr_in maddr,caddr;
	unsigned int port,listennum;
	// fd_set fs;
	// struct timeval tv;
	int retval, maxfd = -1;
	if(argc < 2)
	{
		printf("Usage: %s ip port [listennum]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	if(argv[2])
		port = atoi(argv[2]);
	if(argv[3])
		listennum = atoi(argv[3]);
	else
		listennum = 2;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		handle_error("create socket error!");
	bzero(&maddr, sizeof(maddr));
	maddr.sin_family = AF_INET;
	maddr.sin_port = htons(port);
	if(argv[1])
		maddr.sin_addr.s_addr = inet_addr(argv[1]);
	else
		maddr.sin_addr.s_addr = INADDR_ANY;
	int on = 1;
	if((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) == -1)
		handle_error("setsockopt eror!");
	if((bind(sockfd, (struct sockaddr *) &maddr, sizeof(maddr)) == -1))
		handle_error("bind error!");
	if((listen(sockfd, listennum)) == -1)
		handle_error("listen error!");
	pid_t pid;
	len = sizeof(caddr);
	printf("\033[1;31m***serving***\033[0m\n");
	while(true)
	{
		if((new_fd = accept(sockfd, (struct sockaddr *)&caddr, &len)) == -1)
			handle_error("accept error!");
		else
			printf("\t\033[1;34mclient connect from %s, port %d\033[0m\n",inet_ntoa(caddr.sin_addr),ntohs(caddr.sin_port));
		if((pid = fork()) == -1)
			handle_error("fork error!");
		if(pid == 0)
		{
			close(sockfd);
			serve(new_fd);
			exit(EXIT_SUCCESS);
		}else{
			close(new_fd);
		}
	}
	return 0;
}

void serve(int fd)
{
	char buf[MAXBUF + 1];
	fd_set fs;
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	int maxfd = fd;
	socklen_t len;
	while(true)
	{
		FD_ZERO(&fs);            //need to be cleared before next select
		FD_SET(0,&fs);			 //or it can't detect the change of fd
		FD_SET(fd, &fs);
		int ret = select(maxfd + 1, &fs, NULL, NULL, &tv);
		if(ret == -1)
			handle_error("select error!");
		else if(ret == 0)
			continue;
		else{
			if(FD_ISSET(0, &fs))
			{
				bzero(buf, MAXBUF + 1);
				fgets(buf, MAXBUF, stdin);
				if(!strncasecmp(buf, "quit", 4))
				{
					printf("\033[1;34mThis connection will be closed!\033[0m\n");
					break;
				}
				len = send(fd, buf, strlen(buf) - 1, 0);
				if(len == 0)
				{
					printf("\tsend error!\n");
				}
			}
			if(FD_ISSET(fd, &fs))
			{
				bzero(buf, MAXBUF + 1);
				len = recv(fd, buf, MAXBUF, 0);
				if(len > 0)
				{
					printf("\033[1;32mReceived: \033[0m");
//					fputs("Received: ", stdout);
					fputs(buf, stdout);
					fputs("\n", stdout);
				}
				else if( len < 0){
					printf("\trecv error!\n");
				}else{
					printf("\033[1;34mClient closed!\033[0m\n ");
					break;
				}
			}
		}
	}
}
