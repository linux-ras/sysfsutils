/*
 * sysfs_block.c
 *
 * Generic block utility functions for libsysfs
 *
 * Copyright (C) IBM Corp. 2003
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
#include "libsysfs.h"
#include "sysfs.h"

/**
 * sysfs_close_block_partition: closes a block partition
 * @partition: sysfs_block_partition to close
 */
static void sysfs_close_block_partition(struct sysfs_block_partition *partition)
{
	if (partition != NULL) {
		if (partition->directory != NULL)
			partition->directory = NULL;
	}
}

static void sysfs_del_partition(void *partition)
{
	sysfs_close_block_partition((struct sysfs_block_partition *)partition);
}

/**
 * sysfs_close_block_device: closes a sysfs_block_device
 * @block: sysfs_block_device structure
 */
void sysfs_close_block_device(struct sysfs_block_device *block)
{
	if (block != NULL) {
		if (block->directory != NULL) 
			sysfs_close_directory(block->directory);
		if (block->device != NULL) 
			sysfs_close_device(block->device);
		if (block->partitions  != NULL)
			dlist_destroy(block->partitions);
	}
}

/**
 * alloc_block_device: allocate a sysfs_block_device
 * returns sysfs_block_device or NULL
 */
static struct sysfs_block_device *alloc_block_device()
{
	return (struct sysfs_block_device *)
		calloc(1, sizeof(struct sysfs_block_device));
}

/**
 * open_block_dir: opens a sysfs block directory
 * returns sysfs_directory on success or NULL on error
 */
static struct sysfs_directory *open_block_dir(const unsigned char *name)
{
	unsigned char path[SYSFS_PATH_MAX];
	struct sysfs_directory *directory = NULL;

	if (name == NULL) {
		errno = EINVAL;
		return NULL;
	}

	memset(path, 0, SYSFS_PATH_MAX);
	if ((sysfs_get_mnt_path(path, SYSFS_PATH_MAX)) != 0) {
		dprintf("Error getting sysfs mount path\n");
		return NULL;
	}

	strcat(path, SYSFS_BLOCK_DIR);
	strcat(path, "/");
	strcat(path, name);
	directory = sysfs_open_directory(path);
	if (directory == NULL) {
		dprintf("Block device %s not supported on this system",	name);
		return NULL;
	}
	if ((sysfs_read_directory(directory)) != 0) {
		dprintf("Error reading directory %s\n", directory->path);
		sysfs_close_directory(directory);
		return NULL;
	}
	sysfs_read_all_subdirs(directory);

	return directory;
}

/**
 * alloc_block_partition: alloc a sysfs_block_partition structure
 */
static struct sysfs_block_partition *alloc_block_partition(void)
{
	return(struct sysfs_block_partition *)
			calloc(1, sizeof(struct sysfs_block_partition));
}

/**
 * get_all_block_devices: Retrieves details of block directory
 * @block: sysfs_block_device for which details are required
 * returns 0 on success, -1 on failure
 */
static int get_all_block_devices(struct sysfs_block_device *block)
{
	struct sysfs_directory *cur = NULL;
	struct sysfs_block_partition *part = NULL;

	if (block == NULL || block->directory == NULL) {
		errno = EINVAL;
		return -1;
	}

	dlist_for_each_data(block->directory->subdirs, cur, 
						struct sysfs_directory) {
		switch(strcmp(cur->name, SYSFS_QUEUE_NAME)) {
			case 0:	/* this is the "queue" directory */
				if ((sysfs_read_directory(cur)) < 0) {
					dprintf("Error reading directory %s\n",
							cur->path);
					return -1;
				}
				break;
			default: /* these are the partitions */
				part = alloc_block_partition();
				if (part == NULL) {
					dprintf("calloc failed\n");
					return -1;
				}
				part->directory = cur;
				strcpy(part->name, cur->name);
				if (block->partitions == NULL) 
					block->partitions = 
						dlist_new_with_delete
						(sizeof(struct 
							sysfs_block_partition),
							 sysfs_del_partition);
				dlist_unshift(block->partitions, part);
				break;
		}
	}
	return 0;		
}

/**
 * sysfs_open_block_device: opens the specific block device and all its related
 * 		details as partitions, etc
 * returns sysfs_block_device struct on success and NULL on error
 */
struct sysfs_block_device *sysfs_open_block_device(unsigned char *name)
{
	struct sysfs_block_device *block = NULL;
	struct sysfs_directory *blockdir = NULL;
	struct sysfs_link *curlink = NULL;

	if (name == NULL) {
		errno = EINVAL;
		return NULL;
	}

	block = alloc_block_device();
	if (block == NULL) {
		dprintf("calloc failed\n");
		return NULL;
	}
	strcpy(block->name, name);
	blockdir = open_block_dir(name);
	if (blockdir == NULL) {
		sysfs_close_block_device(block);
		return NULL;
	}
	strcpy(block->path, blockdir->path);
	block->directory = blockdir;
	if ((get_all_block_devices(block)) != 0) {
		dprintf("Error retrieving devices for block %s\n", name);
		sysfs_close_block_device(block);
		return NULL;
	}
	/* check if the "block" device has a link to the physical device */
	if (block->directory->links != NULL) {
		dlist_for_each_data(block->directory->links, curlink,
				struct sysfs_link) {
			block->device = sysfs_open_device(curlink->target);
			if (block->device == NULL) {
				dprintf("Error opening device at %s\n",
						curlink->target);
			}
		}
	}

	return block;
}

/** 
 * sysfs_get_blockdev_attributes: returns attributes for the block device
 * @block: block device for which attribs are to be returned
 */
struct dlist *sysfs_get_blockdev_attributes(struct sysfs_block_device *block)
{
	if (block == NULL || block->directory == NULL) 
		return NULL;
	
	return(block->directory->attributes);
}

/**
 * sysfs_get_blockdev_attr: searches block device's attributes by name
 * @block: block device to look through
 * @name: attribute name to get
 * returns sysfs_attribute reference with success or NULL with error.
 */
struct sysfs_attribute *sysfs_get_blockdev_attr
		(struct sysfs_block_device *block, const unsigned char *name)
{
        struct sysfs_attribute *cur = NULL;

	if (block == NULL || block->directory == NULL
		|| block->directory->attributes == NULL || name == NULL) {
		errno = EINVAL;
		return NULL;
	}

	cur = sysfs_get_directory_attribute(block->directory,
					(unsigned char *)name);
	if (cur != NULL)
		return cur;

	return NULL;
}
		
/** 
 * sysfs_get_partition_attributes: returns attributes for the block 
 * 					device partition
 * @block: block device partition for which attribs are to be returned
 */
struct dlist *sysfs_get_partition_attributes
				(struct sysfs_block_partition *part)
{
	if (part == NULL || part->directory == NULL) 
		return NULL;

	return(part->directory->attributes);
}

/**
 * sysfs_get_queue_attributes: returns attributes for the block device's
 * 				QUEUE parameters. Used to set #of queued
 * 				requests as well as the choice of IO
 * 				scheduler
 * @block: block device for which the attributes are needed.
 */
struct dlist *sysfs_get_queue_attributes(struct sysfs_block_device *block)
{
	struct sysfs_directory *dir = NULL;

	dlist_for_each_data(block->directory->subdirs, dir, 
					struct sysfs_directory) {
		if ((strcmp(dir->name, SYSFS_QUEUE_NAME)) != 0) 
			continue;
		else 
			return (dir->attributes);
	}
	return NULL;
}

/**
 * sysfs_get_iosched_attributes: returns attributes for the block device's
 * 				IOSCHED parameters for the given device
 * @block: block device for which the attributes are needed
 * returns a dlist of iosched attributes. 
 */
struct dlist *sysfs_get_iosched_attributes(struct sysfs_block_device *block)
{
	struct dlist *list = NULL;
	struct sysfs_directory *dir = NULL, *new = NULL;
	unsigned int found = 0;
	
	dlist_for_each_data(block->directory->subdirs, dir, 
					struct sysfs_directory) {
		if ((strcmp(dir->name, SYSFS_QUEUE_NAME)) != 0) 
			continue;
		else {
			found = 1;
			break;
		}
	}
	if (found) {
		/* 
		 * this is the queue directory - read this and the
		 * iosched directory too
		 */
		dlist_for_each_data(dir->subdirs, new, 
						struct sysfs_directory) {
			if ((strcmp(new->name, SYSFS_IOSCHED_NAME)) == 0) 
				if ((sysfs_read_directory(new)) == 0) 
					return new->attributes;
		}
	}
	dprintf("IOSCHED attributes not found\n");
	return NULL;
}
			
