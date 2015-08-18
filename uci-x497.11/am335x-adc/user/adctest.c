#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ADCDEVNAME "/dev/am335xadc"

#define SET_ADC_CHANNELS        _IOW('z', 1, int)

#define GET_ADC_CHAN1            _IOR('z', 2, int*)
#define GET_ADC_CHAN2            _IOR('z', 3, int*)
#define GET_ADC_CHAN3            _IOR('z', 4, int*)
#define GET_ADC_CHAN4            _IOR('z', 5, int*)
#define GET_ADC_CHAN5            _IOR('z', 6, int*)
#define GET_ADC_CHAN6            _IOR('z', 7, int*)
#define GET_ADC_CHAN7            _IOR('z', 8, int*)
#define GET_ADC_CHAN8            _IOR('z', 9, int*)


int main(int argc, char *argv[])
{
    int fd;
    int result;
    int i;
    int sample;

    fd = open(ADCDEVNAME, O_RDWR);
    if (fd == -1) {
        perror("am335xadc open");
        exit(EXIT_FAILURE);
    }

    result = ioctl(fd, SET_ADC_CHANNELS, 8);
    if (result == -1) {
        perror("am335xadc ioctl: SET_ADC_CHANNELS");
        exit(EXIT_FAILURE);
    }

    printf("Sampling channel1 values:\n");
    for (i = 0; i < 10; i++) {
        result = ioctl(fd, GET_ADC_CHAN1, &sample);
        printf("%d ", sample);
    }
    printf("\n");

    close(fd);

    return 0;
}

