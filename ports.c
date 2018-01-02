#ifndef _PORTS_C
#define _PORTS_C

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/byteorder/generic.h>
#include <linux/cred.h>
#include <linux/sched.h>
#include <uapi/linux/in.h>
#include <linux/net.h>

struct portNode {
    bool blockedStatus;
    int proto;
    int direction;
    unsigned short port;
    long access_count;
    struct list_head list;
};

asmlinkage long fw421_reset(void) {

    struct blocked_port *port1, *port2;
    /* Return an error if the user doesn't have root access */
    if(current_uid().val != 0)
        return -EPERM;
    /* Lock the write mutex */
    down_port_write();
    /* For each entry, while maintaining the ability to get to the next node,
       delete the current node */
    list_for_each_entry_safe(port1, port2, &BLOCKED_PORTS, list) {
        list_del(&port1->list);
        kfree(port1);
    }
    /* Unloc the write mutex after deleting everything */
    up_port_write();
    return 0;

}


asmlinkage long fw421_block_port(int proto, int dir, unsigned short port) {

    struct blocked_port *toAdd, *loop;

    /* Returns an error if the params are wrong or the user isn't root */
    int invalid_params = check_invalid_port(proto, dir, port);
    if(invalid_params)
        return invalid_params;

    /* Lock list for reading, and make sure the port isn't already blocked */
    down_port_read();

    list_for_each_entry(loop, &BLOCKED_PORTS, list) {
        if(loop->proto == proto && loop->port == port &&
           loop->direction == dir) {
            return -EEXIST;
        }
    }

    up_port_read();
    /* If the list didn't have the port, make a new one with the correct data */
    toAdd = (struct blocked_port*)kmalloc(sizeof(struct blocked_port), GFP_KERNEL);
    toAdd->access_count = 0;
    toAdd->proto = proto;
    toAdd->direction = dir;
    toAdd->port = port;

    /* Lock the list of blocked ports for writing, then add the new port to it */
    down_port_write();
    list_add(&toAdd->list, &BLOCKED_PORTS);
    up_port_write();
    return 0;
}


smlinkage long fw421_unblock_port(int proto, int dir, unsigned short port) {

    struct blocked_port *port1, *port2;

    /* Returns an error if the params are wrong or the user isn't root */
    int invalid_params = check_invalid_port(proto, dir, port);
    if(invalid_params)
        return invalid_params;

    down_port_write();
    /* Loop through linked list of ports, and delete the given one */
    list_for_each_entry_safe(port1, port2, &BLOCKED_PORTS, list) {
        if(port1->proto == proto && port1->port == port &&
           port1->direction == dir) {

            list_del(&port1->list);
            kfree(port1);
            up_port_write();
            return 0;
        }
    }
    /* If the given port block isn't in the linked list, then return an error */
    up_port_write();
    return -ENOENT;
}


asmlinkage long fw421_query(int proto, int dir, unsigned short port) {

    struct blocked_port *loop;

    /* Returns an error if the params are wrong or the user isn't root */
    int invalid_params = check_invalid_port(proto, dir, port);
    if(invalid_params)
        return invalid_params;

    down_port_read();

    /* Loop through linked list of ports, and return the access count of
       the given one */
    list_for_each_entry(loop, &BLOCKED_PORTS, list) {
        if(loop->proto == proto && loop->port == port &&
           loop->direction == dir) {

            long toReturn = loop->access_count;
            up_port_read();
            return toReturn;
        }
    }
    /* Return an error if the given port isn't blocked */
    up_port_read();
    return -ENOENT;
}

#endif