#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>

#include "../../tracer.h"
#include "../include/trtest.h"

int main(int argc, char *argv[])
{
	int trfd;
	int ttfd;
	pid_t pid;

	pid = getpid();

	trfd = open("/dev/tracer", O_RDWR);
	if (trfd < 0) {
		fprintf(stderr, "Unable to open /dev/tracer\n");
		exit(EXIT_FAILURE);
	}

	ttfd = open("/dev/trtest", O_RDWR);
	if (ttfd < 0) {
		fprintf(stderr, "Unable to open trtest\n");
		exit(EXIT_FAILURE);
	}

	ioctl(trfd, TRACER_ADD_PROCESS, pid);

	ioctl(ttfd, TRTEST_KMALLOC, 1024);

	printf("After kmalloc:\n");
	system("cat /proc/tracer");

	ioctl(ttfd, TRTEST_KFREE);

	printf("After kfree:\n");
	system("cat /proc/tracer");

	ioctl(trfd, TRACER_REMOVE_PROCESS, pid);

	sleep(5);
	close(ttfd);
	close(trfd);

	return 0;
}

