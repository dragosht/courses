#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>

#define AF_TESTP 19
#define PF_TESTP 19

/*
#define NSOCK 2
*/
#define NSOCK 1

int main(int argc, char *argv[])
{
	int s[NSOCK];
	int i;

	system("insmod af_testp.ko");

	for (i = 0; i < NSOCK; i++) {
		s[i] = socket(AF_TESTP, SOCK_DGRAM, 0);
		/*
		s[i] = socket(AF_PACKET, SOCK_DGRAM, 0);
		*/
		assert(s[i] > 0);
	}

	/* ... */

	for (i = 0; i < NSOCK; i++) {
		close(s[i]);
	}

	system("rmmod af_testp");

	return 0;
}

