#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <sys/syscall.h>

#define __NR_FC421_UNBLOCK_FILE 383

int main(int argc, char *argv[]) {

	if(argc < 2){
		printf("Error in fc421_unblock_file.c: Not enough args\n");
		exit(EXIT_FAILURE);
	}
	else if(argc > 2){
		printf("Error in fc421_unblock_file.c: Too many args\n");
		exit(EXIT_FAILURE);
	}

	long error = syscall(__NR_FC421_UNBLOCK_FILE, argv[1]);
	if(error != 0){
		perror("Error in fc421_unblock_file.c");
		exit(EXIT_FAILURE);
	}
    return error;
}