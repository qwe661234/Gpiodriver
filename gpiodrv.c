#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("National Cheng Kung University, Taiwan");
MODULE_DESCRIPTION("Gpiodrv driver");
MODULE_VERSION("0.1");

#define DEV_NAME "gpiodrv"
#define BCM2708_PERI_BASE        0x3F000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

static dev_t gpiodrv_dev = 0;
static struct cdev *gpiodrv_cdev;
static struct class *gpiodrv_class;
unsigned *gpio_mem;

int gpioalloc(int gpio_num)
{
    if (gpio_request(gpio_num, "Allocate GPIO")) {
        printk("GPIO-%d has been allocated\n", gpio_num);
    } else {
        printk("Init GPIO-%d successfully\n", gpio_num);
    }
    printk("Access GPIO-%d \n", gpio_num);
    return 0;
}

int gpio_out(int gpio_num) {
    gpioalloc(gpio_num);
    if (!gpio_mem) {
        gpio_mem = (unsigned *)ioremap(GPIO_BASE, 4096);
    }
    *(gpio_mem + ((gpio_num) / 10)) &= ~(7<<(((gpio_num) % 10)*3));
    *(gpio_mem +((gpio_num)/10)) |=  (1<<(((gpio_num)%10)*3));
    return 0;
}

int gpio_in(int gpio_num) {
    gpioalloc(gpio_num);
    if (!gpio_mem) {
        gpio_mem = (unsigned *)ioremap(GPIO_BASE, 4096);
    }
    *(gpio_mem + ((gpio_num) / 10)) &= ~(7<<(((gpio_num) % 10)*3));
    return 0;
}

int gpio_set(int gpio_num) {
    gpioalloc(gpio_num);
    if (!gpio_mem) {
        gpio_mem = (unsigned *)ioremap(GPIO_BASE, 4096);
    }
    *(gpio_mem + 7) |= 1 << gpio_num;
    return 0;
}
int gpio_clr(int gpio_num) {
    gpioalloc(gpio_num);
    if (!gpio_mem) {
        gpio_mem = (unsigned *)ioremap(GPIO_BASE, 4096);
    }
    *(gpio_mem + ((gpio_num) / 10)) &= ~(7<<(((gpio_num) % 10)*3));
    *(gpio_mem +((gpio_num)/10)) |=  (1<<(((gpio_num)%10)*3));
    *(gpio_mem + 10) |= 1 << gpio_num;
    return 0;
}

int gpio_read_data(int gpio_num) {
    gpioalloc(gpio_num);
    if (!gpio_mem) {
        gpio_mem = (unsigned *)ioremap(GPIO_BASE, 4096);
    }
    *(gpio_mem + ((gpio_num) / 10)) &= ~(7 << (((gpio_num) % 10) * 3));
    printk("13 = %#x", *(gpio_mem + 13));
    return *(gpio_mem + 13) & (1 << gpio_num) ? 1 : 0;
}

static int gpiodrv_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int gpiodrv_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t gpiodrv_read(struct file *file,
                        char *buf,
                        size_t size,
                        loff_t *offset)
{
    return 0;
}

/* write operation is skipped */
static ssize_t gpiodrv_write(struct file *file,
                         const char *buf,
                         size_t size,
                         loff_t *offset)
{
    return 1;
}

static loff_t gpiodrv_device_lseek(struct file *file, loff_t offset, int orig)
{
    loff_t new_pos = 0;
    file->f_pos = new_pos;  
    return new_pos;
}

long gpiodrv_ioctl(struct file* filp, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
    case 0: 
        gpio_out(arg);
        break;
    case 1:
        gpio_clr(arg);
        break;
    case 2:
        gpio_in(arg);
        break;
    case 3:
        return gpio_read_data(arg);
    case 4:
        gpio_set(arg);
        break;
    }

    return 0;
}

const struct file_operations gpiodrv_fops = {
    .owner = THIS_MODULE,
    .read = gpiodrv_read,
    .write = gpiodrv_write,
    .open = gpiodrv_open,
    .release = gpiodrv_release,
    .llseek = gpiodrv_device_lseek,
    .unlocked_ioctl = gpiodrv_ioctl,
};

static int __init init_gpiodrv_dev(void)
{
    int rc = 0;

    // Let's register the device
    // This will dynamically allocate the major number
    rc = alloc_chrdev_region(&gpiodrv_dev, 0, 1, DEV_NAME);

    if (rc < 0) {
        printk(KERN_ALERT
               "Failed to register the gpiodrv char device. rc = %i",
               rc);
        return rc;
    }

    gpiodrv_cdev = cdev_alloc();
    if (gpiodrv_cdev == NULL) {
        printk(KERN_ALERT "Failed to alloc cdev");
        rc = -1;
        goto failed_cdev;
    }
    gpiodrv_cdev->ops = &gpiodrv_fops;
    rc = cdev_add(gpiodrv_cdev, gpiodrv_dev, 1);

    if (rc < 0) {
        printk(KERN_ALERT "Failed to add cdev");
        rc = -2;
        goto failed_cdev;
    }

    gpiodrv_class = class_create(THIS_MODULE, DEV_NAME);

    if (!gpiodrv_class) {
        printk(KERN_ALERT "Failed to create device class");
        rc = -3;
        goto failed_class_create;
    }

    if (!device_create(gpiodrv_class, NULL, gpiodrv_dev, NULL, DEV_NAME)) {
        printk(KERN_ALERT "Failed to create device");
        rc = -4;
        goto failed_device_create;
    }
    return rc;
failed_device_create:
    class_destroy(gpiodrv_class);
failed_class_create:
    cdev_del(gpiodrv_cdev);
failed_cdev:
    unregister_chrdev_region(gpiodrv_dev, 1);
    return rc;
}

static void __exit exit_gpiodrv_dev(void)
{
    device_destroy(gpiodrv_class, gpiodrv_dev);
    class_destroy(gpiodrv_class);
    cdev_del(gpiodrv_cdev);
    unregister_chrdev_region(gpiodrv_dev, 1);
}

module_init(init_gpiodrv_dev);
module_exit(exit_gpiodrv_dev);