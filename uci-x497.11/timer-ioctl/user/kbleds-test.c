#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define KBLEDS "/dev/kbleds"

#define MAGIC 'z'
#define GETDELAY _IOR(MAGIC, 1, int*)
#define SETDELAY _IOW(MAGIC, 2, int*)
#define GETLEDS  _IOR(MAGIC, 3, int*)
#define SETLEDS  _IOW(MAGIC, 4, int*)

int main(int argc, char *argv[])
{
    int fd;
    int delay;
    int leds;
    int result;

    unsigned char cleds;

    fd = open(KBLEDS, O_RDWR);
    if (fd == -1) {
        perror("kbleds open");
        exit(EXIT_FAILURE);
    }

    if (argc < 3) {
        fprintf(stderr,
                "Usage: %s <delay> <leds>\n"
                "<delay> - time between LEDs blinks (msecs)\n"
                "<leds> - LEDS to blink 0x00 - 0x07 (3 LEDs)\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    delay = atoi(argv[1]);
    leds = atoi(argv[2]);

    result = ioctl(fd, SETDELAY, &delay);
    if (result == -1) {
        perror("kbleds ioctl: SETDELAY");
        exit(EXIT_FAILURE);
    }

    result = ioctl(fd, SETLEDS, &leds);
    if (result == -1) {
        perror("kbleds ioclt: SETLEDS");
        exit(EXIT_FAILURE);
    }

    result = ioctl(fd, GETDELAY, &delay);
    if (result == -1) {
        perror("kbleds ioctl: GETDELAY");
        exit(EXIT_FAILURE);
    }

    result = ioctl(fd, GETLEDS, &leds);
    if (result == -1) {
        perror("kbleds ioctl: GETLEDS");
        exit(EXIT_FAILURE);
    }

    printf("kbleds ioctl results: delay: %d, leds: %d\n",
            delay, leds);

    read(fd, &cleds, 1);
    printf("kbleds read results: leds: %d\n", cleds);


    close(fd);

    return 0;
}

