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

struct socket_cut {
    socket_state state;
    short type;
    unsigned long flags;
};

struct message_to_user { 
    struct vfsmount_cut vfs_cut;
    struct socket_cut socket_cut;
};

struct message_to_kernel {
    int fd;
    int socketfd;
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

    int socketfd = socket(AF_INET, SOCK_STREAM, 0);

    char* deviceFile = "/dev/etx_device";
    int driver = open(deviceFile, O_RDWR);
    if (driver != 0) {
        printf("failed to open device %s\n", deviceFile);
        return 1;
    }

    struct message_to_kernel kmsg = {
        .fd = fd,
        .socketfd = socketfd
    };
    if (ioctl(driver, WR_VALUE, (message_to_kernel *) &kmsg)) {
        puts("failed to write to driver");
    }

    struct message_to_user msg;
    if (ioctl(driver, RD_VALUE, (struct message_to_user *) &msg)) {
        puts("failed to read structs from driver");
    }

    struct vfsmount_cut vc = msg.vfs_cut;
    struct socket_cut sc = msg.socket_cut;

    puts(" --- vfsmount --- ");
    printf("Flags: %#0x\n", vc.mnt_flags);
    printf("Block size in bits: %u\n", vc.s_blocksize_bits);
    printf("Block size in bytes: %lu\n", vc.s_blocksize);
    printf("Reference count: %d\n", vc.s_count);
    printf("Maximum size of files: %ld\n", vc.s_maxbytes);
    printf("Device identifier: %lu\n", vc.s_dev);
    puts(" --- socket --- ");

    char* sstates[] = {
        "not allocated",
        "unconnected to any socket",
        "in process of connecting",
        "connected to socket",
        "in process of disconnecting"
    };

    char* stypes[] = {
        "",
        "SOCK_DGRAM",	    
	    "SOCK_STREAM",
	    "SOCK_RAW",	   
	    "SOCK_RDM",	    
	    "SOCK_SEQPACKET",
	    "SOCK_DCCP", 
        "", "", "",
	    "SOCK_PACKET"
    }; 

    printf("Socket state: %s\n", sstates[sc.state]);
    printf("Socket type: %s\n", stypes[sc.type])
    printf("Socket flags: %#0x\n", sc.flags);

    close(fd); 
    close(socketfd);
    close(driver);

    return 0;
}