#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/rwsem.h>
#include <linux/list.h>
#include <linux/namei.h>
#include <linux/fs.h>
#include <linux/cred.h>
#include <linux/sched.h>
#include <linux/dcache.h>
#include "file_block.h"

static DEFINE_MUTEX(READ_LOCK);
static DEFINE_MUTEX(WRITE_LOCK);
static volatile int num_readers;

static LIST_HEAD(BLOCKED_FILES);

void down_file_read(void) {
	/* Lock the mutex guarding the number of readers */
	mutex_lock(&READ_LOCK);
	/* Increment the reader counter, then lock 
	   writer mutex if it's the first reader, 
	   so writers can't write while there's still readers */
	if(++num_readers == 1) {
		mutex_lock(&WRITE_LOCK);
	}
	/* Unlock mutex guarding reader number */
	mutex_unlock(&READ_LOCK);
}

void up_file_read(void) {
	/* Lock the mutex guarding the number of readers */
	mutex_lock(&READ_LOCK);
	/* Subtract one from the reader counter, then 
	   unlock writer mutex if it was the last reader */
	if(--num_readers == 0) {
		mutex_unlock(&WRITE_LOCK);
	}
	/* Unlock mutex guarding reader number */
	mutex_unlock(&READ_LOCK);
}

void down_file_write(void) {
	mutex_lock(&WRITE_LOCK);
}

void up_file_write(void) {
	mutex_unlock(&WRITE_LOCK);
}

int path_to_inode(const char* __user path_name, ino_t* inode) {
	struct path has_inode;
	int path_len, error;
	char* from_user;

	/* Make sure the path name isn't NULL and user is root */
	if(path_name == NULL)
		return -EINVAL;
	if(current_uid().val != 0)
		return -EPERM;

	/* String is a user pointer, need to convert it to kernel pointer */
	path_len = strlen_user(path_name);
	from_user = (char*)kmalloc(path_len, GFP_KERNEL);
	/* If there's an issue converting the path, return -EFAULT */
	if(copy_from_user(from_user, path_name, path_len)) {
		kfree(from_user);
		return -EFAULT;
	}
	/* Get the path string's associated path struct */
	error = kern_path(from_user, LOOKUP_FOLLOW, &has_inode);
	/* Don't need the string anymore, so free that up */
	kfree(from_user);
	/* If there was an error with the path trace, return taht error */
	if(error)
		return error;
	/* Set the ino_t value to the path's final inode number and return */
	*inode = has_inode.dentry->d_inode->i_ino;
	return 0;
}

int check_invalid_inode(const struct inode *inode) {

	struct blocked_file *loop;
	if(inode == NULL)
		return 0;

	down_file_read();
	/* If the list is empty, return OK */
	if(list_empty(&BLOCKED_FILES)){
		up_file_read();
		return 0;
	}

	/* Loop through the list, check if the inode value is in it */
	list_for_each_entry(loop, &BLOCKED_FILES, list) {
		/* If it is, add one to it's access count and return invalid */
		if(loop->file_id == inode->i_ino) {
			up_file_read();
			down_file_write();
			loop->access_count++;
			up_file_write();

			return -EINVAL;
		}
	}
	/* If the inode isn't in the list, return OK */
	up_file_read();
	return 0;

}

asmlinkage long fc421_reset(void) {
	
	struct blocked_file *file1, *file2;

	/* Gotta check root access */
	if(current_uid().val != 0)
		return -EPERM;

	/* Loop through entire list, freeing each entry as it goes */
	down_file_write();
	list_for_each_entry_safe(file1, file2, &BLOCKED_FILES, list) {
		list_del(&file1->list);
		kfree(file1);
	}
	up_file_write();
	return 0;
}

asmlinkage long fc421_block_file(const char *filename) {
	
	struct blocked_file *toAdd, *loop;
	ino_t inode;
	int error;

	/* Get the filename's inode number, return an error if it fails */
	error = path_to_inode(filename, &inode);
	if(error) {
		return error;
	}
	/* Check to see if the file is already in the list. If so, return an error */
	down_file_read();
	list_for_each_entry(loop, &BLOCKED_FILES, list) {
		if(loop->file_id == inode) {
			up_file_read();
			return -EEXIST;
		}
	}
	up_file_read();

	/* Make a blocked_file struct and fill it with the right data */
	toAdd = (struct blocked_file*)kmalloc(sizeof(struct blocked_file), GFP_KERNEL);
	toAdd->access_count = 0;
	toAdd->file_id = inode;

	/* Add it to the blocked_file linked list */
	down_file_write();
	list_add(&toAdd->list, &BLOCKED_FILES);
	up_file_write();
	return 0;
}

asmlinkage long fc421_unblock_file(const char *filename) {

	struct blocked_file *file1, *file2;
	ino_t inode;
	int error;

	/* Get the inode number of the file from the file path */
	error = path_to_inode(filename, &inode);
	if(error)
		return error;

	down_file_write();
	/* If the list is empty, return an error */
	if(list_empty(&BLOCKED_FILES)) {
		up_file_write();
		return -ENOENT;
	}
	/* Otehrwise, loop through the list and 
	   delete the given files' inode from it */
	list_for_each_entry_safe(file1, file2, &BLOCKED_FILES, list) {
		if(file1->file_id == inode) {
			list_del(&file1->list);
			kfree(file1);
			up_file_write();
			return 0;
		}	
	}
	/* If it's not in the list, return an error */
	up_file_write();
	return -ENOENT;
}

asmlinkage long fc421_query(const char *filename) {

	struct blocked_file *loop;
	ino_t inode;
	int error;

	/* Get the inode number of the file from the file path */
	error = path_to_inode(filename, &inode);
	if(error)
		return error;

	down_file_read();
	/* Loop through linked list to see if the inode matches with anything.
	   If it does, return the access count of the file */
	list_for_each_entry(loop, &BLOCKED_FILES, list) {

		if(loop->file_id == inode) {
			long toReturn = loop->access_count;
			up_file_read();
			return toReturn;
		}
	}
	/* If the linked list doesn't match anything, return -ENOENT */
	up_file_read();
	return -ENOENT;
}