#ifndef _FILE_BLOCK_H
#define _FILE_BLOCK_H

struct blocked_file {
	ino_t file_id;
	long access_count;
	struct list_head list;
};

/* Locks blocked_file linked list for reading */
void down_file_read(void);

/* Locks blocked_file linked list for writing */
void up_file_read(void);

/* Locks blocked_file linked list for writing */
void down_file_write(void);

/* Unlocks blocked_file linked list for writing */
void up_file_write(void);

/* Converts the given path into its corresponding inode value, 
   only callable by the root user */
int path_to_inode(const char* __user path_name, ino_t* inode);

/* Checks to see if the inode is blocked. If it is, it returns -EINVAL.
   Otherwise, it returns 0. */
int check_invalid_inode(const struct inode *inode);

/* Syscall to reset the blocked file linked list */
asmlinkage long fc421_reset(void);

/* Syscall to add a file name to the blocked file linked list */
asmlinkage long fc421_block_file(const char *filename);

/* Syscall to remove a file name from the blocked file linked list */
asmlinkage long fc421_unblock_file(const char *filename);

/* Syscall to get the number of times a blocked file has been accessed */
asmlinkage long fc421_query(const char *filename);

#endif