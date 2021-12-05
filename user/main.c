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

struct message { 
    struct vfsmount_cut vfs_cut;
};


int main(int argc, char *argv[]) {
    char* filename = argv[1];
    int fd = open(filename, O_RDONLY);
    if(fd < 0) return 1;

    printf("\nOpening Driver\n");
    int driverfd = open("/dev/etx_device", O_RDWR);
    if (driverfd < 0) {
        printf("Cannot open device file...\n");
        return 1;
    }

    printf("Writing fd to Driver\n");
    ioctl(driverfd, WR_VALUE, (int32_t *) &fd); // todo change type

    struct message msg;
    printf("Reading Value from Driver\n");
    ioctl(driverfd, RD_VALUE, (struct message *) &msg);

    printf("Flags: %#0x\n", msg.vfs_cut.mnt_flags);
    printf("Block size in bits: %u\n", msg.vfs_cut.s_blocksize_bits);
    printf("Block size in bytes: %lu\n", msg.vfs_cut.s_blocksize);
    printf("Reference count: %d\n", msg.vfs_cut.s_count);
    printf("Maximum size of files: %ld\n", msg.vfs_cut.s_maxbytes);
    printf("Device identifier: %lu\n", msg.vfs_cut.s_dev);

    close(fd);
    close(driverfd);

    return 0;















    // int fd;
    // int32_t value = atoi(argv[1]);

    // struct message msg;

    // printf("\nOpening Driver\n");
    // fd = open("/dev/etx_device", O_RDWR);
    // if (fd < 0) {
    //     printf("Cannot open device file...\n");
    //     return 0;
    // }


    // printf("Writing Pid to Driver\n");
    // ioctl(fd, WR_VALUE, (int32_t *) &value);

    // printf("Reading Value from Driver\n");
    // ioctl(fd, RD_VALUE, (struct message *) &msg);

    // printf("Signals info:\n");
    // printf("Threads number in current process is %d\n", msg.si.nr_threads);
    // printf("Priority of current process is %d\n", msg.si.prio);
    // printf("Flags of signals are %d\n", msg.si.flags);

    // int i;

    // for (i = 0; i < 64; i++)
    //     printf("%d - 0x%08X\n", i, (unsigned int) msg.si.sigHandlersAddr[i]);

    // printf("_________________________\n");
    // printf("Net info:\n");

    // for (int c = 0; c < msg.ndi.size; c++) {
    //     printf("found device %d [%s]\n", c + 1, msg.ndi.name[c]);
    // }

    // printf("Closing Driver\n");
    // close(fd);
}