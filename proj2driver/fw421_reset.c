#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/kernel.h>
#include <sys/syscall.h>

#define __NR_FW421_RESET 377

int main(int argc, char *argv[]) {

	long error = syscall(__NR_FW421_RESET);
	if(error != 0){
		perror("Error in fw421_reset.c");
		exit(EXIT_FAILURE);
	}
    return error;
}