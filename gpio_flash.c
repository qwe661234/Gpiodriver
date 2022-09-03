#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define GPIO_DEV "/dev/gpiodrv"

int main()
{
    char buf[1];
    char write_buf[] = "testing writing";

    int fd = open(GPIO_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    int gpio_num = 21;

    int count = 0;
    while(++count != 10) {
        ioctl(fd, 0, 2);
        sleep(1);
        ioctl(fd, 1, 2);
        sleep(1);

    }
    
    close(fd);
    return 0;
}