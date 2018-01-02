#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <sys/syscall.h>

#define __NR_FC421_RESET 381

int main(int argc, char *argv[]) {

	long error = syscall(__NR_FC421_RESET);
	if(error != 0){
		perror("Error in fc421_reset.c");
		exit(EXIT_FAILURE);
	}
    return error;
}