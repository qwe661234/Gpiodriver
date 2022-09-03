#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define GPIO_DEV "/dev/gpiodrv"
#define GPIO_OUT 0
#define GPIO_CLR 1
#define GPIO_IN 2
#define GPIO_READ_DATA 3
#define GPIO_SET 4


int main()
{
    char buf[1];
    char write_buf[] = "testing writing";

    int fd = open(GPIO_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    int gpio_input_num = 21;
    int gpio_output_nums[3] = {2, 3, 4};

    ioctl(fd, GPIO_IN, gpio_input_num);
    for(int i = 0; i < 3; i++) {
        ioctl(fd, GPIO_OUT, gpio_output_nums[i]);
    }

    while(1) {
        if (ioctl(fd, GPIO_READ_DATA, gpio_input_num)) {
            for(int i = 0; i < 3; i++) {
                ioctl(fd, GPIO_SET, gpio_output_nums[i]);
            }
        } else {
            for(int i = 0; i < 3; i++) {
                ioctl(fd, GPIO_CLR, gpio_output_nums[i]);
            }
        }
    }

    close(fd);
    return 0;
}