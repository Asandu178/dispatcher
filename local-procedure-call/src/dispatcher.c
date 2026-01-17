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
#define CONNECT_PIPE_C ".dispatcher/connection_req_pipe"
#define BACKLOG 400
#define POOL 40
#define DETACHED 1

int static epollfd;

typedef struct {
    char Version[20];
    char CallPipeName[200];
    char ReturnPipeName[200];
    char AccessPath[200];
} ServiceDetails;

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
    int fd = *(int *)args;
    printf("%d\n", fd);
    fflush(stdout);
    w_epoll_update_fd_out(epollfd, fd);
    return NULL;
}

void handle_install(char *buf) {
    
    uint16_t numar = *(uint16_t*)buf;
    numar = be16toh(numar);
    char* text = (char*)(buf + 2);

    printf("%d\n", numar);
    printf("%s\n", text);
    fflush(stdout);
    
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

    ret = makePipe(INSTALL_PIPE_C, 0777);
    if (ret < 0)
    {
        perror("Failed to create install pipe"); 
        exit(1);
    }

    //Creare pipe pentru connect
    ret = makePipe(CONNECT_PIPE_C, 0777);
    if (ret < 0)
    {
        perror("Failed to create connect pipe");
        exit(1);
    }

	epollfd = w_epoll_create();
	int installfd = open(INSTALL_PIPE_C, O_RDONLY | O_NONBLOCK);
	int connectionfd = open(CONNECT_PIPE_C, O_RDONLY | O_NONBLOCK);
	w_epoll_add_fd_in(epollfd, installfd);
	w_epoll_add_fd_in(epollfd, connectionfd);

    printf("Pipe-urile au fost create cu succes!\n");
    fflush(stdout);


	while(1) {
		struct epoll_event res;
		w_epoll_wait_infinite(epollfd, &res);

		if (res.events & EPOLLIN) {
			if (res.data.fd == installfd) {
                char *buf = calloc(200, 1);
                int size = read(installfd, buf, 200);
                handle_install(buf);
                //write(STDOUT_FILENO, buf, size);
            }
			if (res.data.fd == connectionfd) {
                char *buf = calloc(200, 1);
                int size = read(installfd, buf, 200);
                write(STDOUT_FILENO, buf, size);
            }
		}
	}
    return 0;
}