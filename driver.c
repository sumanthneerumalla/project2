#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/kernel.h>
#include <sys/syscall.h>

#define __IPPROTO_TCP 6
#define __IPPROTO_UDP 17


int main() {

    int errno = syscall(377); /* test the reset funcion */
    if(errno != 0){
        perror("error in fw421_reset function");
        exit(EXIT_FAILURE);
    }

    errno = syscall(378); /* test the query function */

    /* test the block function with these arguments */
    errno = syscall(379, __IPPROTO_TCP, 0, 1000);
    if(errno != 0){
        perror("error from blocking port:");
        exit(EXIT_FAILURE);
    }

    /* test the unblock function with these arguments */
    errno = syscall(380, __IPPROTO_TCP, 0, 1000);
    if(errno != 0){
        perror("error from un blocking port:");
        exit(EXIT_FAILURE);
    }

    errno = syscall(381, __IPPROTO_TCP, 0, 1000);
    if(errno != 0){
        perror("error from querying blocked port:");
        exit(EXIT_FAILURE);
    }


}