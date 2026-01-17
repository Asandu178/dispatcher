// SPDX-License-Identifier: BSD-3-Clause

#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void makePipe(const char *path, int mode) {
	int rc = 0;
	rc = access(path, F_OK);
	if (rc < 0)
		rc = mkfifo(path, mode);
	
	return rc;
}

int main(void)
{
	return 0;
}
