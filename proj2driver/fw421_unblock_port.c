#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <strings.h>

#define __NR_FW421_UNBLOCK_PORT 379
#define __IPPROTO_TCP 6
#define __IPPROTO_UDP 17

int main(int argc, char *argv[]) {

	int proto, dir;
	unsigned short port;

	if(argc < 4){
		printf("Error in fw421_unblock_port.c: Not enough args\n");
		exit(EXIT_FAILURE);
	}
	else if(argc > 4){
		printf("Error in fw421_unblock_port.c: Too many args\n");
		exit(EXIT_FAILURE);
	}

	if(strcasecmp(argv[1], "tcp") == 0) {
		proto = __IPPROTO_TCP;
	}
	else if(strcasecmp(argv[1], "udp") == 0) {
		proto = __IPPROTO_UDP;
	}
	else {
		printf("Error in fw421_unblock_port.c: Unrecognized protocol (Use tcp/udp)\n");
		exit(EXIT_FAILURE);
	}

	if(strcasecmp(argv[2], "in")) {
		dir = 1;
	}
	else if(strcasecmp(argv[2], "out")) {
		dir = 0;
	}
	else {
		printf("Error in fw421_unblock_port.c: Unrecognized direction (Use in/out)\n");
		exit(EXIT_FAILURE);
	}
	port = (unsigned short)atoi(argv[3]);

	long error = syscall(__NR_FW421_UNBLOCK_PORT, proto, dir, port);
	if(error != 0){
		perror("Error in fw421_unblock_port.c");
		exit(EXIT_FAILURE);
	}
    return error;
}