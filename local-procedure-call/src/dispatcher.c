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
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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

void *handle_install(void *ctx) {
    pthread_mutex_lock(&lock);
    int installfd = *(int *)ctx;
    char *buf = calloc(200, 1);
    int size = read(installfd, buf, 200);
    
    uint16_t numar = *(uint16_t*)buf;
    numar = be16toh(numar);
    char *text = calloc(numar, 1);
    strcpy(text, (buf + 2));

    if (makePipe(text, 0777) < 0) {
        printf("Failed to make pipe\n");
    }

    int fd = open(text, O_RDONLY | O_CREAT, 0644);

    char installBuf[0xff] = {0};

    int x = read(fd, installBuf, sizeof(installBuf));
    printf("%d\n", x);

    uint8_t m_VersionLen = *(uint8_t *)installBuf;
    u_int16_t m_CpnLen = be16toh(*(uint16_t *)(installBuf + 1));
    u_int16_t m_RpnLen = be16toh(*(uint16_t *)(installBuf + 3));
    u_int16_t m_ApLen = be16toh(*(uint16_t *)(installBuf + 5));
    char *content = installBuf + 7;
        printf("%s\n", content);

    char *version = calloc(m_VersionLen + 1, 1);
    char *call_pipe = calloc(m_CpnLen + 1, 1);
    char *return_pipe = calloc(m_RpnLen + 1, 1);
    char *access_path = calloc(m_ApLen + 1, 1);
    
    strncpy(version, content, m_VersionLen);
    strncpy(call_pipe, content + m_VersionLen + m_VersionLen, m_CpnLen);
    strncpy(return_pipe, content + m_CpnLen + m_VersionLen, m_RpnLen);
    strncpy(access_path, content + m_CpnLen + m_RpnLen + m_VersionLen, m_ApLen);

    printf("%d | %d | %d | %d\n", m_VersionLen, m_CpnLen, m_RpnLen, m_ApLen);
    printf("%s | %s | %s | %s\n", version, call_pipe, return_pipe, access_path);

    printf("%d\n", numar);
    printf("%s\n", text);
    fflush(stdout);
    close(fd);
    pthread_mutex_unlock(&lock);
    
}

void *handle_connection(void *ctx) {
    pthread_mutex_lock(&lock);
    int connectionfd = *(int *)ctx;

    char *buf = calloc(200, 1);
    int size = read(connectionfd, buf, 200);
    printf("test\n");
    fflush(stdout);
    pthread_mutex_unlock(&lock);
}

int main(void)
{
    int ret;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, DETACHED);


    if (mkdir(DISPATCHER_C, 0777) < 0 && access(DISPATCHER_C, F_OK) < 0) {
        perror("mkdir dispatcher");
    }
    
    if (mkdir(PIPES_C, 0777) < 0 && access(PIPES_C, F_OK) < 0) {
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
        pthread_t test;
		w_epoll_wait_infinite(epollfd, &res);

		if (res.events & EPOLLIN) {
			if (res.data.fd == installfd) {
                pthread_create(&test, &attr, handle_install, &installfd);
            }
			if (res.data.fd == connectionfd) {
                pthread_create(&test, &attr, handle_connection, &connectionfd);
            }
		} else {

        }
	}
    return 0;
}