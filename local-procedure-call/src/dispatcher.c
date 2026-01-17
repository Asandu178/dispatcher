#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#define DISPATCHER_C ".dispatcher"
#define PIPES_C ".pipes"
#define INSTALL_PIPE_C ".dispatcher/install_req_pipe"
#define CONNECT_PIPE_C ".dispatcher/connect_req_pipe"

int makePipe(const char *path, int mode)
{
    int rc = 0;
    // Verificăm dacă există deja
    if (access(path, F_OK) < 0) {
        rc = mkfifo(path, mode);
    }
    return rc;
}

int main(void)
{
    int ret;

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

    printf("Pipe-urile au fost create cu succes!\n");
    return 0;
}