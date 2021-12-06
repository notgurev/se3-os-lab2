#include <sys/ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/net.h>
#include <stdlib.h>                                                                                
#include <string.h>                                                                                     
#include <stdio.h>                                                                                      
#include <unistd.h>                                                                                     
#include <sys/types.h>                                                                                  
#include <sys/socket.h>                                                                                 
#include <netinet/in.h> 

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

#define MNT_NOSUID    0x01
#define MNT_NODEV    0x02
#define MNT_NOEXEC    0x04
#define MNT_NOATIME    0x08
#define MNT_NODIRATIME    0x10
#define MNT_RELATIME    0x20
#define MNT_READONLY    0x40
#define MNT_NOSYMFOLLOW    0x80
#define MNT_SHRINKABLE  0x100
#define MNT_WRITE_HOLD  0x200
#define MNT_SHARED  0x1000 
#define MNT_UNBINDABLE  0x2000  

#define flagdef(name, value) flags[value] = #name

char *flags[MNT_UNBINDABLE + 1];

void init_flags() {
    flagdef(MNT_NOSUID, 0x01);
    flagdef(MNT_NODEV, 0x02);
    flagdef(MNT_NOEXEC, 0x04);
    flagdef(MNT_NOATIME, 0x08);
    flagdef(MNT_NODIRATIME, 0x10);
    flagdef(MNT_RELATIME, 0x20);
    flagdef(MNT_READONLY, 0x40);
    flagdef(MNT_NOSYMFOLLOW, 0x80);
    flagdef(MNT_SHRINKABLE,  0x100);
    flagdef(MNT_WRITE_HOLD,  0x200);
    flagdef(MNT_SHARED,  0x1000);
    flagdef(MNT_UNBINDABLE , 0x2000);  
}

void print_flags(int number) {
    printf("Flags: ");
    for (int mask = 1; mask <= MNT_UNBINDABLE; mask = mask << 1) {
        if ((number & mask) != 0) {
            printf("%s ", flags[mask]);
        }
    }
    puts("");
}

int main(int argc, char *argv[]) {
    if (argc < 1) {
        puts("wrong number of arguments, expected 1");
    }
    char* filename = argv[1];
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("failed to open file %s\n", filename);
        return -1;
    }

    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
        puts("failed to create socket");
        return -1;
    }

    char* deviceFile = "/dev/etx_device";
    int driver = open(deviceFile, O_RDWR);
    if (driver < 0) {
        printf("failed to open device %s\n", deviceFile);
        return -1;
    }

    struct message_to_kernel kmsg = {
        .fd = fd,
        .socketfd = socketfd
    };
    if (ioctl(driver, WR_VALUE, (struct message_to_kernel *) &kmsg)) {
        puts("failed to write to driver");
    }

    struct message_to_user msg;
    if (ioctl(driver, RD_VALUE, (struct message_to_user *) &msg)) {
        puts("failed to read structs from driver");
    }

    struct vfsmount_cut vc = msg.vfs_cut;
    struct socket_cut sc = msg.socket_cut;

    init_flags();

    puts(" --- vfsmount --- ");
    printf("Flags (hex): %0x\n", vc.mnt_flags);
    print_flags(vc.mnt_flags);
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
    printf("Socket type: %s\n", stypes[sc.type]);
    printf("Socket flags: %#0lx\n", sc.flags);

    close(fd); 
    close(socketfd);
    close(driver);

    return 0;
}
