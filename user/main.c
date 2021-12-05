#include <sys/ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <malloc.h>

#define WR_VALUE _IOW('a','a',struct message*)
#define RD_VALUE _IOR('a','b',struct message*)


struct signal_info {
    int nr_threads;
    int prio;
    unsigned int flags;
    unsigned int sigHandlersAddr[64];
};

struct n_dev_info {
    unsigned int size;
    char name[10][16];
};

struct message {
    struct signal_info si;
    struct n_dev_info ndi;
};


int main(int argc, char *argv[]) {
    int fd;
    //struct bpf_redirect_info_short *bpf;
    int32_t value = atoi(argv[1]);

    struct message msg;

    printf("\nOpening Driver\n");
    fd = open("/dev/etx_device", O_RDWR);
    if (fd < 0) {
        printf("Cannot open device file...\n");
        return 0;
    }


    printf("Writing Pid to Driver\n");
    ioctl(fd, WR_VALUE, (int32_t *) &value);

    printf("Reading Value from Driver\n");
    ioctl(fd, RD_VALUE, (struct message *) &msg);

    printf("Signals info:\n");
    printf("Threads number in current process is %d\n", msg.si.nr_threads);
    printf("Priority of current process is %d\n", msg.si.prio);
    printf("Flags of signals are %d\n", msg.si.flags);

    int i;

    for (i = 0; i < 64; i++)
        printf("%d - 0x%08X\n", i, (unsigned int) msg.si.sigHandlersAddr[i]);

    printf("_________________________\n");
    printf("Net info:\n");

    for (int c = 0; c < msg.ndi.size; c++) {
        printf("found device %d [%s]\n", c + 1, msg.ndi.name[c]);
    }

    printf("Closing Driver\n");
    close(fd);

}