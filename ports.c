#ifndef _PORTS_C
#define _PORTS_C

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <uapi/linux/in.h>
#include <linux/net.h>

struct blockedNode {
    bool blockedStatus;
    int type;
    int direction;
    int portNumber;
    int count;
    struct list_head list;
};

/*borrowed from netinet/in.h, used to check the type of block being done*/
#define IPPROTO_IP              0               /* dummy for IP */
#define IPPROTO_UDP             17              /* user datagram protocol */
#define IPPROTO_TCP             6               /* tcp */


/*using define mutex instead of initialize mutex because allows for global mutexes */
static DEFINE_MUTEX(noRead);
static DEFINE_MUTEX(noWrite);
static int waitingProcesses;
static LIST_HEAD(blacklistedPorts);


asmlinkage long fw421_reset(void) {
    /* use a mutex in order to prevent multiple port blocks from affecting each other*/
    mutex_lock(&noWrite);

    struct blockedNode *first; /*temp  node used to iterate through list and delete items*/
    list_for_each_entry(first, &blacklistedPorts, list) {
        first->count = first->count++;
        list_del(&first->list);/*delete and then free the memory*/
        kfree(first);
    }

    mutex_unlock(&noWrite);/*exit critical section, allow other processes to also use list*/
    return 0;

}


int checkBadUser(){
    if(current_uid().val != 0){
        return 2;
   }
}


blockedNode* initNode(protocol,direction,portNumber,status){

    struct blockedNode *temp;
    temp = (struct blockedNode*)kmalloc(sizeof(struct blockedNode), GFP_KERNEL);
    temp->blockedStatus = status;
    temp->count = 0;
    temp->proto = protocol;
    temp->direction = direction;
    temp->portNumber = portNumber;

    return temp;
}

asmlinkage long fw421_block_port(int proto, int dir, unsigned short port) {

   /*error checking accepts only the right block requests*/
    if(proto != IPPROTO_TCP && proto != IPPROTO_UDP && proto !=IPPROTO_IP){
        return -EINVAL;}
    if(direction != 0 && direction != 1){
        return -EINVAL;}


    struct blockedNode *temp, *target;

    /*search for the node, return error if it is already blocked*/
    list_for_each_entry(target, &blacklistedPorts, list) {
        target->count = target->count++;
        if(target->proto == proto && target->portNumber == port && target->direction == dir) {
            return -EINVAL;
        }}
    temp = initNode(proto,dir,port,True);
    list_add(&temp->list, &blacklistedPorts);
    return 0;
}


smlinkage long fw421_unblock_port(int proto, int dir, unsigned short port) {

    struct blockedNode *temp;

    /*error checking accepts only the right block requests*/
    if(proto != IPPROTO_TCP && proto != IPPROTO_UDP && proto !=IPPROTO_IP){
        return -EINVAL;}
    if(direction != 0 && direction != 1){
        return -EINVAL;}

    list_for_each_entry(temp,&blacklistedPorts, list) {

        if(temp->proto == proto && temp->port == port && temp->direction == dir) {
            list_del(&temp->list);
            kfree(temp);
            up_port_write();
            return 0;
        }}
    return -EINVAL;/*errof if not found*/
}


asmlinkage long fw421_query(int proto, int dir, unsigned short port) {

    struct blockedNode *temp;

    /*error checking accepts only the right block requests*/
    if(proto != IPPROTO_TCP && proto != IPPROTO_UDP && proto !=IPPROTO_IP){
        return -EINVAL;}
    if(direction != 0 && direction != 1){
        return -EINVAL;}

    list_for_each_entry(temp, &blacklistedPorts, list) {
        temp->count = temp->count++;
        if(temp->proto == proto && temp->portNumber == port) {
            return temp->count;/*return the count once found int he list)*/
        }}
    return -EINVAL;/*otherwise return an error*/
}

#endif