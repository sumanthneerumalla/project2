#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <strings.h>

#define __NR_FW421_QUERY 380
#define __IPPROTO_TCP 6
#define __IPPROTO_UDP 17


int main(int argc, char *argv[]) {

	int proto, dir;
	unsigned short port;

	if(argc < 4){
		printf("Error in fw421_query.c: Not enough args\n");
		exit(EXIT_FAILURE);
	}
	else if(argc > 4){
		printf("Error in fw421_query.c: Too many args\n");
		exit(EXIT_FAILURE);
	}

	if(strcasecmp(argv[1], "tcp") == 0) {
		proto = __IPPROTO_TCP;
	}
	else if(strcasecmp(argv[1], "udp") == 0) {
		proto = __IPPROTO_UDP;
	}
	else {
		printf("Error in fw421_query.c: Unrecognized protocol (Use tcp/udp)\n");
		exit(EXIT_FAILURE);
	}

	if(strcasecmp(argv[2], "in")) {
		dir = 1;
	}
	else if(strcasecmp(argv[2], "out")) {
		dir = 0;
	}
	else {
		printf("Error in fw421_query.c: Unrecognized direction (Use in/out)\n");
		exit(1);
	}
	port = (unsigned short)atoi(argv[3]);

	long response = syscall(__NR_FW421_QUERY, proto, dir, port);
	if(response < 0){
		perror("Error in fw421_query syscall");
		exit(EXIT_FAILURE);
	}
	printf("The port is blocked, and has been accessed %d times.\n", response);
    return 0;
}