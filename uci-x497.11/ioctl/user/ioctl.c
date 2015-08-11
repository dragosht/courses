/*
 *  ioctl_user_space.c - User space IOCTL calls to Kernel space driver 
 *              
 */
#include<fcntl.h>
#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<sys/ioctl.h>


#define MAGIC 'z'
#define GETIOCTL _IOR(MAGIC, 1, int *)
#define SETIOCTL _IOW(MAGIC, 2, int *)
#define HELLOPLUS "/dev/helloplus"

int main(int argc, char* argv[])
{
    int fd;
    int temp_sec;
    unsigned char seconds = 5;

    int result;

    fd = open(HELLOPLUS,O_RDWR);
    if(fd == -1)
    {
        printf("Error in open\n");
        return -1;
    }

    if (argc < 2)
    {
        printf("How many seconds between messages? ");
        scanf("%d", &temp_sec);
    } else {
        temp_sec = atoi(argv[1]);
    }
    seconds = temp_sec;

    result = ioctl(fd, SETIOCTL, &seconds);
    if(result == -1)
    {
        printf("Error in ioctl\n");
        return -1;
    }
    printf("The result of SETIOCTL: %d\n",seconds);


    result = ioctl(fd, GETIOCTL, &seconds);
    if(result == -1)
    {
        printf("Error in ioctl\n");
        return -1;
    }
    printf("The result of GETIOCTL: %d\n",seconds);

    printf("\n");
    return 0;
}
