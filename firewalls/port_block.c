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

#include "port_block.h"

static DEFINE_MUTEX(READ_LOCK);
static DEFINE_MUTEX(WRITE_LOCK);
static volatile int num_readers;
static LIST_HEAD(BLOCKED_PORTS);

void down_port_read(void) {
	/* Lock the mutex guarding the number of readers */
	mutex_lock(&READ_LOCK);
	/* Increment the reader counter, then lock 
	   writer mutex if it's the first reader, 
	   so writers can't write while there's still
	   readers */
	if(++num_readers == 1) {
		mutex_lock(&WRITE_LOCK);
	}
	/* Unlock mutex guarding reader number */
	mutex_unlock(&READ_LOCK);
}

void up_port_read(void) {
	/* Lock reader num mutex */
	mutex_lock(&READ_LOCK);
	/* Subtract one from the reader counter, then 
	   unlock writer mutex if it was the last reader */
	if(--num_readers == 0) {
		mutex_unlock(&WRITE_LOCK);
	}
	/* Unlock mutex guarding reader number */
	mutex_unlock(&READ_LOCK);
}

void down_port_write(void) {
	mutex_lock(&WRITE_LOCK);
}

void up_port_write(void) {
	mutex_unlock(&WRITE_LOCK);
}

int check_invalid_port(int proto, int direction, unsigned short port) {
	/* Return an error if the user doesn't have root access */
	if(current_uid().val != 0)
		return -EPERM;
	/* If the protocol isn't TCP or UDP, then return invalid */
	if(proto != IPPROTO_TCP && proto != IPPROTO_UDP)
		return -EINVAL;
	/* Direction should be either 0 or 1, nothing else */
	if(direction != 0 && direction != 1)
		return -EINVAL;
	return 0;
}

/* Called in system calls, gets port number from struct, converts it to 
   CPU-native endianness, then calls is_port_blocked */
int is_port_blocked(int proto, int direction, struct sockaddr_storage *umyaddr) {
	unsigned short port;
	struct sockaddr_in *getPort;
	struct blocked_port *loop;

	/* Convert protocol stored in the socket to the appropriate protocol name,
	   If it's not tcp or udp, return OK */
	if(proto == SOCK_STREAM)
		proto = IPPROTO_TCP;
	else if(proto == SOCK_DGRAM)
		proto = IPPROTO_UDP;
	else
		return 0;
	/* Make sure the given address isn't NULL */
	if(umyaddr == NULL)
		return -EINVAL;

	/* Get port number from sockaddr struct, and turn it to system endianness */
	getPort = (struct sockaddr_in*)umyaddr;
	port = (short int)be16_to_cpup(&getPort->sin_port);

	down_port_read();
	/* If there's nothing in the list, return OK */
	if(list_empty(&BLOCKED_PORTS)){
		up_port_read();
		return 0;
	}
	/* Otherwise, loop through the list and see if it's blocked */
	list_for_each_entry(loop, &BLOCKED_PORTS, list) {

		if(loop->proto == proto && loop->port == port && 
			loop->direction == direction) {
			/* If it is, increment access count and return an error */
			up_port_read();

			down_port_write();
			loop->access_count++;
			up_port_write();
			return -EINVAL;
		}
	}
	/* If it's not blocked, return OK */
	up_port_read();
	return 0;
}

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

asmlinkage long fw421_unblock_port(int proto, int dir, unsigned short port) {

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