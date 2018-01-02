#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/list.h>

#include <linux/namei.h>

struct fileNode {/* struct used to define nodes in the linked list of blocked files*/
    ino_t fileIdentifier;
    int count;
    struct list_head list;
};


/*using define mutex instead of initialize mutex because allows for global mutexes */
/* borrowed from my ports file*/
static DEFINE_MUTEX(noRead);
static DEFINE_MUTEX(noWrite);
static LIST_HEAD(blockedFiles);


asmlinkage long fc421_reset(void) {/* very similar to approach used in ports file*/

    /* use a mutex in order to prevent multiple port blocks from affecting each other*/
    mutex_lock(&noWrite);

    struct fileNode *first; /*temp  node used to iterate through list and delete items*/
    list_for_each_entry(first, &blockedFiles, list) {
        first->count = first->count++;
        list_del(&first->list);/*delete and then free the memory*/
        kfree(first);
    }

    mutex_unlock(&noWrite);/*exit critical section, allow other processes to also use list*/
    return 0;

}


/* takes in information about the node to add to the block list, and returns a struct pointer*/
fileNode* initNode(protocol,direction,portNumber,status){

    struct blockedNode *temp;
    temp = (struct fileNode*)kmalloc(sizeof(struct fileNode), GFP_KERNEL);
    temp->blockedStatus = status;
    temp->count = 0;
    temp->fileIdentifier = inode;

    return temp;

}


asmlinkage long fc421_block_file(const char *filename) {

    struct fileNode *temp, *loopNode;
    ino_t inode;
    /* try to get filename's inode number, return the error if there is one */
    int error = path_to_inode(filename, &inode);
    if(error != NULL && error != 0) {
        return error;
    }

    /*search for the node, return error if it is already blocked*/
    list_for_each_entry(loopNode, &blockedFiles, list) {
        if(loopNode->fileIdentifier == inode) {
            up_file_read();
            return -EINVAL;
        }
    }

    list_add(&temp->list, &blockedFiles);/* add node to linked list*/
    return 0;
}

asmlinkage long fc421_unblock_file(const char *filename) {

    struct fileNode *temp;
    ino_t inode;


    /* try to get filename's inode number, return the error if there is one */
    int error = path_to_inode(filename, &inode);
    if(error != NULL && error != 0) {
        return error;
    }

   list_for_each_entry(temp, &blockedFiles, list) {
        if(temp->fileIdentifier == inode) {
            list_del(&temp->list);
            return 0;
        }
    }
    return -EINVAL; /* if its not in there, then return an error*/
}

asmlinkage long fc421_query(const char *filename) {

    struct fileNode *temp;
    ino_t inode;
    mutex_lock(noWrite);
    /* try to get filename's inode number, return the error if there is one */
    int error = path_to_inode(filename, &inode);
    if(error != NULL && error != 0) {
        return error;
    }

    list_for_each_entry(temp, &blockedFiles, list) {
        if(temp->fileIdentifier == inode) {
            int toReturn = temp->count;
        }
    }
    mutex_unlock(noWrite);
    return -EINVAL;/*if nothing was found just throw an error*
}