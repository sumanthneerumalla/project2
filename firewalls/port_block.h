
#ifndef _PORT_BLOCK_H
#define _PORT_BLOCK_H

struct blocked_port {
	int proto;
	int direction;
	unsigned short port;
	long access_count;
	struct list_head list;
};

/* Locks blocked_port linked list for reading */
void down_port_read(void);

/* Unlocks blocked_port linked list for reading */
void up_port_read(void);

/* Locks blocked_port linked list for writing */
void down_port_write(void);

/* Unlocks blocked_port linked list for writing */
void up_port_write(void);

/* Does basic error validation for system calls requiring root privledges */
int check_invalid_port(int proto, int direction, unsigned short port);

/* Checks whether the socket address is in the linked list. Returns -EINVAL 
   If it is. */
int is_port_blocked(int proto, int direction, struct sockaddr_storage *umyaddr);

/* Deletes everything in the linked list, "resetting" the port blocking */
asmlinkage long fw421_reset(void);

/* Adds the given port to the linked list on success, returns an error on failure */
asmlinkage long fw421_block_port(int proto, int dir, unsigned short port);

/* Removes the given port from the linked list, returns an error on failure */
asmlinkage long fw421_unblock_port(int proto, int dir, unsigned short port);

/* Looks for the given port in the linked list, returns how many times it was accessed on
   success. returns an error on failure. */
asmlinkage long fw421_query(int proto, int dir, unsigned short port);

#endif