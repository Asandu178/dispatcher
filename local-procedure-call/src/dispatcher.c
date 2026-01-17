#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "./protocol/w_epoll.h"
#include <pthread.h>

#define DISPATCHER_C ".dispatcher"
#define PIPES_C ".pipes"
#define INSTALL_PIPE_C ".dispatcher/install_req_pipe"
#define CONNECT_PIPE_C ".dispatcher/connect_req_pipe"
#define BACKLOG 400
#define POOL 40
#define DETACHED 1

int static epollfd;
pthread_t threads[POOL];

int makePipe(const char *path, int mode)
{
    int rc = 0;
    // Verificăm dacă există deja
    if (access(path, F_OK) < 0) {
        rc = mkfifo(path, mode);
    }
    return rc;
}

void *execute(void *args) {
	
}

int main(void)
{
    int ret;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, DETACHED);


    if (mkdir(DISPATCHER_C, 0755) < 0 && access(DISPATCHER_C, F_OK) < 0) {
        perror("mkdir dispatcher");
    }
    
    if (mkdir(PIPES_C, 0755) < 0 && access(PIPES_C, F_OK) < 0) {
        perror("mkdir pipes");
    }

    // Creare pipe pentru install
    ret = makePipe(INSTALL_PIPE_C, 0777);
    if (ret < 0)
    {
        perror("Failed to create install pipe"); 
        exit(1);
    }

    // Creare pipe pentru connect
    ret = makePipe(CONNECT_PIPE_C, 0777);
    if (ret < 0)
    {
        perror("Failed to create connect pipe");
        exit(1);
    }

	epollfd = w_epoll_create();
	int installfd = open(INSTALL_PIPE_C, O_RDONLY);
	int connectionfd = open(CONNECT_PIPE_C, O_RDONLY);
	w_epoll_add_fd_in(epollfd, installfd);
	w_epoll_add_fd_in(epollfd, connectionfd);

    printf("Pipe-urile au fost create cu succes!\n");

	while(1) {
		struct epoll_event res;
		w_epoll_wait_infinite(epollfd, &res);
		if (res.events & EPOLLIN) {
			pthread_create(NULL, &attr, execute, NULL);
		}
	}
    return 0;
}