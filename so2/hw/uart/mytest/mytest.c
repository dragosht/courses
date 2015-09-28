#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

#include "../uart16550.h"

int do_read(int fd, unsigned char *buffer, int size)
{
	int n, from=0;
	
	while (1) {
		n=read(fd, &buffer[from], size-from);
		if (n <= 0)
		        return -1;
	        if (n+from == size)
	    	        return 0;
		from+=n;
	}
}

int do_write(int fd, unsigned char *buffer, unsigned int size)
{
	int n, from=0;
	
	while (1) {
		n=write(fd, &buffer[from], size-from);
		if (n <= 0) {
			perror("write");
		        return -1;
		}
		if (n+from == size)
			return 0;
		from+=n;
	}
}

unsigned char tbuf[] = "abcdefghijklmnopqrstuvwxyz";

int main(int argc, char *argv[])
{
	unsigned char buffer[1024];

	struct uart16550_line_info uli;
	uli.baud = UART16550_BAUD_9600;
	uli.len = UART16550_LEN_8;
	uli.stop = UART16550_STOP_1;
	uli.par  = UART16550_PAR_NONE;

	int fd0 = open("/dev/uart0", O_WRONLY);
	int fd1 = open("/dev/uart1", O_RDONLY);


	assert(fd0 >= 0);
	assert(fd1 >= 0);

	assert(ioctl(fd0, UART16550_IOCTL_SET_LINE, &uli) == 0);
	assert(ioctl(fd1, UART16550_IOCTL_SET_LINE, &uli) == 0);

	memset(buffer, 0, 1024);

	do_write(fd0, tbuf, sizeof(tbuf));
	do_read(fd1, buffer, sizeof(tbuf));

	printf("read: %s\n", buffer);

	close(fd0);
	close(fd1);

	return 0;
}

