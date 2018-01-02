#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/kernel.h>
#include <sys/syscall.h>

#define __NR_FC421_QUERY 384

int main(int argc, char *argv[]) {

	if(argc < 2){
		printf("Error in fw421_query.c: Not enough args\n");
		exit(EXIT_FAILURE);
	}
	else if(argc > 2){
		printf("Error in fw421_query.c: Too many args\n");
		exit(EXIT_FAILURE);
	}

	long response = syscall(__NR_FC421_QUERY, argv[1]);
	if(response < 0){
		perror("Error in fc421_query syscall");
		exit(EXIT_FAILURE);
	}
	printf("The port is blocked, and has been accessed %d times.\n", response);
    return 0;


}