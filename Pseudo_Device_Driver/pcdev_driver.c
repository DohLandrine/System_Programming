#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "pcdev"
#define BUFFER_SIZE 512

char device_buffer[BUFFER_SIZE];    //the pseudo memory that simulates hardware

dev_t dev_number;                   // Device number (major + minor)
struct cdev pcdev_cdev;             // Character device structure
loff_t current_position = 0;        // Current Read/Write position

// definition of open file operation

static int pcdev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "pcdev: Device opened\n");
    return 0;
}

static int pcdev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "pcdev: Device closed\n");
    return 0;
}

// definition of read file operation
static ssize_t pcdev_read(struct file *file, char __user *buffer, size_t length, loff_t *offset)
{
    if (*offset >= BUFFER_SIZE) {
        return 0; // End of file
    }

    if ((*offset + length) > BUFFER_SIZE) {
        length = BUFFER_SIZE - *offset;
    }

    if (copy_to_user(buffer, device_buffer + *offset, length)) {
        return -EFAULT; // Error in copying data to user space
    }

    *offset += length;
    printk(KERN_INFO "pcdev: Read %zu bytes from device\n", length);
    return length;
}

// definition of write file operation

static ssize_t pcdev_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset)
{
    if (*offset >= BUFFER_SIZE) {
        return -ENOSPC; // No space left on device
    }

    if ((*offset + length) > BUFFER_SIZE) {
        length = BUFFER_SIZE - *offset;
    }

    if (copy_from_user(device_buffer + *offset, buffer, length)) {  // 
        return -EFAULT; // Error in copying data from user space
    }

    *offset += length;
    printk(KERN_INFO "pcdev: Written %zu bytes to device\n", length);
    return length;
}

// definition of seek support

static loff_t pcdev_lseek(struct file *file, loff_t offset, int whence)
{
    loff_t new_position = 0;

    switch (whence){
        case SEEK_SET: new_position = offset; break;
        case SEEK_END: new_position = BUFFER_SIZE +offset; break;
        case SEEK_CUR: new_position = file -> f_pos + offset; break;
        default: return -EINVAL; // Invalid whence
    }

        if (new_position < 0 || new_position > BUFFER_SIZE) {
            return -EINVAL; // Invalid seek position
        }

        file->f_pos = new_position;
        return new_position;    
}

// Register the device;
static struct file_operations pcdev_fops = {
    .owner = THIS_MODULE,
    .open = pcdev_open,
    .release = pcdev_release,
    .read = pcdev_read,
    .write = pcdev_write,
    .llseek = pcdev_lseek,
};
// Module initialization function
static int __init pcdev_init(void)
{
    alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
    cdev_init(&pcdev_cdev, &pcdev_fops);
    cdev_add(&pcdev_cdev, dev_number, 1);
    printk(KERN_INFO "pcdev driver loaded, device: /dev/%s\n", DEVICE_NAME);
    return 0;
}

static void __exit pcdev_exit(void){
    cdev_del(&pcdev_cdev);
    unregister_chrdev_region(dev_number, 1);
    printk(KERN_INFO "pcdev driver unloaded\n");
}

module_init(pcdev_init);
module_exit(pcdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Doh Landrine");
MODULE_DESCRIPTION("Pseudo Character Device Driver");