/*
 * SO2 - Lab 6 - Deferred Work
 *
 * Exercises #3, #4, #5: deferred work
 *
 * User-mode test program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "../include/deferred.h"

#define DEVICE_PATH	"/dev/deferred"

/* prints error message and exits */
void error(char *message)
{
	perror(message);
	exit(EXIT_FAILURE);
}

/* prints usage message and exits */
void usage()
{
	printf("Usage: test <options>\n options:\n"
			"\ts <seconds> - set timer to run after <seconds> seconds\n"
			"\tc           - cancel timer\n"
			"\ta <seconds> - allocate memory after <seconds> seconds\n"
			"\n");
	exit(1);
}

int main(int argc, char **argv)
{
	int fd;
	unsigned long seconds;

	if (argc < 2)
		usage();

	fd = open(DEVICE_PATH, O_RDONLY);
	if (fd < 0)
		error(DEVICE_PATH);

	switch (argv[1][0]) {
	case 's':
		/* set timer */
		if (argc < 3)
			usage();
		seconds = atoi(argv[2]);
		printf("Set timer to %ld seconds\n", seconds);
		if (ioctl(fd, MY_IOCTL_TIMER_SET, seconds) < 0)
			error("Ioctl set timer error");
		break;
	case 'c':
		/* cancel timer */
		printf("Cancel timer\n");
		if (ioctl(fd, MY_IOCTL_TIMER_CANCEL) < 0)
			error("Ioctl cancel timer error");
		break;
	case 'a':
		/* allocate memory */
		if (argc < 3)
			usage();
		seconds = atoi(argv[2]);
		printf("Allocate memory after %ld seconds\n",seconds);
		if (ioctl(fd, MY_IOCTL_TIMER_ALLOC, seconds) < 0)
			error("Ioctl allocate memory error");
		break;
	default:
		error("Wrong parameter");
	}

	close(fd);

	return 0;
}

