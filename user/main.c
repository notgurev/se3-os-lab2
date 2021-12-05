#include <sys/ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <malloc.h>

#include <unistd.h>
#include <fcntl.h>

#define WR_VALUE _IOW('a','a',struct message*)
#define RD_VALUE _IOR('a','b',struct message*)

struct vfsmount_cut {
    int mnt_flags;
    unsigned char s_blocksize_bits;
    unsigned long s_blocksize;
    int s_count;
    loff_t s_maxbytes;
    dev_t s_dev;
};

struct net_device_cut {

};

struct message { 
    struct vfsmount_cut vfs_cut;
    struct net_device_cut nd_cut;
};

int main(int argc, char *argv[]) {
    if (argc < 1) {
        puts("wrong number of arguments, expected 1")
    }
    char* filename = argv[1];
    int fd = open(filename, O_RDONLY);
    if (fd != 0) {
        printf("failed to open file %s\n", filename);
        return 1;
    }

    char* deviceFile = "/dev/etx_device";
    int driver = open(deviceFile, O_RDWR);
    if (driver != 0) {
        printf("failed to open device %s\n", deviceFile);
        return 1;
    }

    if (ioctl(driver, WR_VALUE, (int *) &fd)) {
        puts("failed to write to driver");
    }

    struct message msg;
    if (ioctl(driver, RD_VALUE, (struct message *) &msg)) {
        puts("failed to read structs from driver");
    }

    printf("Flags: %#0x\n", msg.vfs_cut.mnt_flags);
    printf("Block size in bits: %u\n", msg.vfs_cut.s_blocksize_bits);
    printf("Block size in bytes: %lu\n", msg.vfs_cut.s_blocksize);
    printf("Reference count: %d\n", msg.vfs_cut.s_count);
    printf("Maximum size of files: %ld\n", msg.vfs_cut.s_maxbytes);
    printf("Device identifier: %lu\n", msg.vfs_cut.s_dev);

    close(fd); 
    close(driver);

    return 0;
}