/*
 * test_class.c
 *
 * Tests for class related functions for the libsysfs testsuite
 *
 * Copyright (C) IBM Corp. 2004
 *
 *      This program is free software; you can redistribute it and/or modify it
 *      under the terms of the GNU General Public License as published by the
 *      Free Software Foundation version 2 of the License.
 *
 *      This program is distributed in the hope that it will be useful, but
 *      WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License along
 *      with this program; if not, write to the Free Software Foundation, Inc.,
 *      675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/**
 ***************************************************************************
 * this will test the class related functions provided by libsysfs.
 *
 * extern void sysfs_close_class_device(struct sysfs_class_device *dev);
 * extern struct sysfs_class_device *sysfs_open_class_device_path
 * 					(const unsigned char *path);
 * extern struct sysfs_class_device *sysfs_open_class_device
 * 	(const unsigned char *class, const unsigned char *name);
 * extern struct sysfs_device *sysfs_get_classdev_device
 * 				(struct sysfs_class_device *clsdev);
 * extern struct sysfs_driver *sysfs_get_classdev_driver
 * 				(struct sysfs_class_device *clsdev);
 * extern struct sysfs_class_device *sysfs_get_classdev_parent
 *				(struct sysfs_class_device *clsdev);
 * extern void sysfs_close_class(struct sysfs_class *cls);
 * extern struct sysfs_class *sysfs_open_class(const unsigned char *name);
 * extern struct dlist *sysfs_get_class_devices(struct sysfs_class *cls);
 * extern struct sysfs_class_device *sysfs_get_class_device
 * 			(struct sysfs_class *class, unsigned char *name);
 * extern struct dlist *sysfs_get_classdev_attributes
 * 				(struct sysfs_class_device *cdev);
 * extern struct dlist *sysfs_refresh_classdev_attributes
 * 				(struct sysfs_class_device *cdev);
 * extern struct sysfs_attribute *sysfs_get_classdev_attr
 * 	(struct sysfs_class_device *clsdev, const unsigned char *name);
 * extern struct sysfs_attribute *sysfs_open_classdev_attr
 * 			(const unsigned char *classname, 
 * 			const unsigned char *dev, 
 * 			const unsigned char *attrib); 
 *****************************************************************************
 */

#include "test.h"
#include <errno.h>

/** 
 * this test the function:
 *   extern void sysfs_close_class_device(struct sysfs_class_device *dev);
 *   this has no return, 
 *
 *   flag:
 *   	0:	dev -> valid
 *   	1:	dev -> NULL
 */
int test_sysfs_close_class_device(int flag)
{
	struct sysfs_class_device *dev = NULL;
	char *path = NULL;

	switch (flag) {
	case 0:
		path = val_class_dev_path;
		dev = sysfs_open_class_device_path(path);
		if (dev == NULL) {
			dbg_print("%s: failed opening class device at %s\n",
					__FUNCTION__, path);
			return 0;
		}
		break;
	case 1:
		dev = NULL;
		break;
	default:
		return -1;
	}
	sysfs_close_class_device(dev);

	dbg_print("%s: returns void\n", __FUNCTION__);
	return 0;
}

/**
 * extern struct sysfs_class_device *sysfs_open_class_device_path
 * 					(const unsigned char *path);
 * flag:
 * 	0:	path -> valid
 * 	1:	path -> invalid
 * 	2:	path -> NULL
 */
int test_sysfs_open_class_device_path(int flag)
{
	struct sysfs_class_device *dev = NULL;
	char *path = NULL;

	switch (flag) {
	case 0:
		path = val_class_dev_path;
		break;
	case 1:
		path = inval_path;
		break;
	case 2:
		path = NULL;
		break;
	default:
		return -1;
	}
	dev = sysfs_open_class_device_path(path);
	
	switch (flag) {
	case 0:
		if (dev == NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_class_device(dev);
			dbg_print("\n");
		}
		break;
	case 1:
	case 2:
		if (dev != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
	default:
		break;

	}
	if (dev != NULL)
		sysfs_close_class_device(dev);
	
	return 0;
}

/**
 * extern struct sysfs_class_device *sysfs_open_class_device
 * 	(const unsigned char *class, const unsigned char *name);
 *
 * flag:
 * 	0:	class -> valid , name -> valid
 * 	1:	class -> valid , name -> invalid
 * 	2:	class -> valid , name -> NULL
 * 	3:	class -> invalid , name -> valid
 * 	4:	class -> invalid , name -> invalid
 * 	5:	class -> invalid , name -> NULL
 * 	6:	class -> NULL valid , name -> valid
 * 	7:	class -> NULL , name -> invalid
 * 	8:	class -> NULL , name -> NULL
 */
int test_sysfs_open_class_device(int flag)
{
	struct sysfs_class_device *clsdev = NULL;
	char *name = NULL;
	char *class = NULL;

	switch(flag) {
	case 0:
		class = val_class;
		name = val_class_dev;
		break;
	case 1:
		class = val_class;
		name = inval_name;
		break;
	case 2:
		class = val_class;
		name = NULL; 
		break;
	case 3:
		class = inval_name;
		name = val_class_dev;
		break;
	case 4:
		class = inval_name;
		name = inval_name;
		break;
	case 5:
		class = inval_name;
		name = NULL; 
		break;
	case 6:
		class = NULL;
		name = val_class_dev;
		break;
	case 7:
		class = NULL;
		name = inval_name;
		break;
	case 8:
		class = NULL;
		name = NULL; 
		break;
	default:
		return -1;
	}
	clsdev = sysfs_open_class_device(class, name);
	
	switch(flag) {
	case 0:
		if (clsdev == NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_class_device(clsdev);
			dbg_print("\n");
		}
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		if (clsdev != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		break;
	default:
		break;
	}
	if (clsdev != NULL)
		sysfs_close_class_device(clsdev);
	
	return 0;
}

/**
 * extern struct sysfs_device *sysfs_get_classdev_device
 * 				(struct sysfs_class_device *clsdev);
 *
 * flag:
 * 	0:	clsdev -> valid
 * 	1:	clsdev -> NULL
 */
int test_sysfs_get_classdev_device(int flag)
{
	struct sysfs_class_device *clsdev = NULL;
	struct sysfs_device *dev = NULL;
	char *path = NULL;

	switch(flag) {
	case 0:
		path = val_class_dev_path;
		clsdev = sysfs_open_class_device_path(path);
		if (clsdev == NULL) {
			dbg_print("%s: failed opening class device at %s\n",
					__FUNCTION__, path);
			return 0;
		}
		break;
	case 1:
		clsdev = NULL;
		break;
	default:
		return -1;
	}
	dev = sysfs_get_classdev_device(clsdev);

	switch (flag) {
	case 0:
		if (dev == NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_device(dev);
			dbg_print("\n");
		}
		break;
	case 1:
		if (dev != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		break;
	default:
		break;
	}
	if (clsdev != NULL)
		sysfs_close_class_device(clsdev);

	return 0;
}

/**
 * extern struct sysfs_driver *sysfs_get_classdev_driver
 * 				(struct sysfs_class_device *clsdev);
 *
 * flag:
 * 	0:	clsdev -> valid
 * 	1:	clsdev -> NULL
 */
int test_sysfs_get_classdev_driver(int flag)
{
	struct sysfs_class_device *clsdev = NULL;
	struct sysfs_driver *drv = NULL;
	char *path = NULL;

	switch(flag) {
	case 0:
		path = val_class_dev_path;
		clsdev = sysfs_open_class_device_path(path);
		if (clsdev == NULL) {
			dbg_print("%s: failed opening class device at %s\n",
					__FUNCTION__, path);
			return 0;
		}
		break;
	case 1:
		clsdev = NULL;
		break;
	default:
		return -1;
			
	}
	drv = sysfs_get_classdev_driver(clsdev);

	switch (flag) {
	case 0:
		if (drv == NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_driver(drv);
			dbg_print("\n");
		}
		break;
	case 1:
		if (drv != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		break;
	default:
		break;
	}
	if (clsdev != NULL)
		sysfs_close_class_device(clsdev);

	return 0;
}

/**
 * extern struct sysfs_class_device *sysfs_get_classdev_parent
 *				(struct sysfs_class_device *clsdev);
 * flag:
 * 	0:	clsdev -> valid
 * 	1:	clsdev -> NULL
 */
int test_sysfs_get_classdev_parent(int flag)
{
	struct sysfs_class_device *clsdev = NULL;
	struct sysfs_class_device *parent = NULL;
	char *path = NULL;

	switch(flag) {
	case 0:
		path = val_block_class_dev_path;
		clsdev = sysfs_open_class_device_path(path);
		if (clsdev == NULL) {
			dbg_print("%s: failed opening class device at %s\n",
					__FUNCTION__, path);
			return 0;
		}
		break;
	case 1:
		clsdev = NULL;
		break;
	default:
		return -1;
	}
	parent = sysfs_get_classdev_parent(clsdev);

	switch (flag) {
	case 0:
		if (parent == NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_class_device(parent);
			dbg_print("\n");
		}
		break;
	case 1:
		if (parent != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		break;
	default:
		break;
	}
	if (clsdev != NULL)
		sysfs_close_class_device(clsdev);

	return 0;
}

/**
 * extern void sysfs_close_class(struct sysfs_class *cls);
 *
 * flag:
 * 	0:	cls -> valid
 * 	1:	cls -> NULL
 */
int test_sysfs_close_class(int flag)
{
	struct sysfs_class *cls = NULL;
	char *class = NULL;

	switch(flag) {
	case 0:
		class = val_class;
		cls = sysfs_open_class(class);
		if (cls == NULL) {
			dbg_print("%s: failed opening class device at %s\n",
					__FUNCTION__, class);
			return 0;
		}
		break;
	case 1:
		cls = NULL;
		break;
	default:
		return -1;
	}
	sysfs_close_class(cls);
	dbg_print("%s: returns void\n", __FUNCTION__);

	return 0;
}
 
/** 
 * extern struct sysfs_class *sysfs_open_class(const unsigned char *name);
 *
 * flag:
 * 	0:	name -> valid
 * 	1:	name -> invalid
 * 	2:	name -> NULL
 */
int test_sysfs_open_class(int flag)
{
	struct sysfs_class *cls = NULL;
	char *name = NULL;

	switch(flag) {
	case 0:
		name = val_class;
		break;
	case 1:
		name = inval_name;
		break;
	case 2:
		name = NULL;
		break;
	default:
		return -1;
	}
	cls = sysfs_open_class(name);

	switch(flag) {
	case 0:
		if (cls == NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			dbg_print("Class %s is at %s\n\n",
					cls->name, cls->path);
		}
		break;
	case 1:
	case 2:
		if (cls != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		break;
	default:
		break;
	}
	if (cls != NULL)
		sysfs_close_class(cls);

	return 0;
}

/**
 * extern struct dlist *sysfs_get_class_devices(struct sysfs_class *cls);
 *
 * flag:
 * 	0:	cls -> valid
 * 	1:	cls -> NULL
 */
int test_sysfs_get_class_devices(int flag)
{
	struct sysfs_class *cls = NULL;
	struct dlist *list = NULL;
	char *class = NULL;

	switch(flag) {
	case 0:
		class = val_class;
		cls = sysfs_open_class(class);
		if (cls == NULL) {
			dbg_print("%s: failed opening class device at %s\n",
					__FUNCTION__, class);
			return 0;
		}
		break;
	case 1:
		cls = NULL;
		break;
	default:
		return -1;
			
	}
	list = sysfs_get_class_devices(cls);
	
	switch(flag) {
	case 0:
		if (list == NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_class_device_list(list);
			dbg_print("\n");
		}
		break;
	case 1:
		if (list != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		break;
	default:
		break;
	}
	if (cls != NULL)
		sysfs_close_class(cls);

	return 0;
}

/** 
 * extern struct sysfs_class_device *sysfs_get_class_device
 * 	(struct sysfs_class *class, unsigned char *name);
 *
 * flag:
 * 	0:	class -> valid, name -> valid
 * 	1:	class -> valid, name -> invalid
 * 	2:	class -> valid, name -> NULL
 * 	3:	class -> NULL, name -> valid
 * 	4:	class -> NULL, name -> invalid
 * 	5:	class -> NULL, name -> NULL
 */
int test_sysfs_get_class_device(int flag)
{
	struct sysfs_class_device *clsdev = NULL;
	struct sysfs_class *class = NULL;
	char *name = NULL;

	switch(flag) {
	case 0:
		class = sysfs_open_class(val_class);
		if (class == NULL) {
			dbg_print("%s: failed opening class %s\n",
					__FUNCTION__, val_class);
			return 0;
		}
		name = val_class_dev;
		break;
	case 1:
		class = sysfs_open_class(val_class);
		if (class == NULL) {
			dbg_print("%s: failed opening class %s\n",
					__FUNCTION__, val_class);
			return 0;
		}
		name = inval_name;
		break;
	case 2:
		class = sysfs_open_class(val_class);
		if (class == NULL) {
			dbg_print("%s: failed opening class %s\n",
					__FUNCTION__, val_class);
			return 0;
		}
		name = NULL;
		break;
	case 3:
		class = NULL;
		name = val_class_dev;
		break;
	case 4:
		class = NULL;
		name = inval_name;
		break;
	case 5:
		class = NULL;
		name = NULL;
		break;
	default:
		return -1;
	}
	clsdev = sysfs_get_class_device(class, name);
	
	switch(flag) {
	case 0:
		if (clsdev == NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_class_device(clsdev);
			dbg_print("\n");
		}
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		if (clsdev != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
	default:
		break;
	}
	if (class)
		sysfs_close_class(class);

	return 0;
}

/** 	
 * extern struct dlist *sysfs_get_classdev_attributes
 * 			(struct sysfs_class_device *cdev);
 * flag:
 * 	0:	cdev -> valid 
 * 	1:	cdev -> NULL 
 */
int test_sysfs_get_classdev_attributes(int flag)
{
	struct dlist *list = NULL;
	struct sysfs_class_device *cdev = NULL;
	char *path = NULL;

	switch (flag) {
	case 0:
		path = val_class_dev_path;
		cdev = sysfs_open_class_device_path(path);
		if (cdev == NULL) {
			dbg_print("%s: failed opening class device at %s\n",
					__FUNCTION__, path);
			return 0;
		}
		break;
	case 1:
		cdev = NULL;
		break;
	default:
		return -1;
	}	
	list = sysfs_get_classdev_attributes(cdev);

	switch (flag) {
	case 0:
		if (list == NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_attribute_list(list);
			dbg_print("\n");
		}
		break;
	case 1:
		if (list != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
	default:
		break;
	}
	if (cdev != NULL)
		sysfs_close_class_device(cdev);

	return 0;
}

/**
 * extern struct dlist *sysfs_refresh_classdev_attributes
 * 				(struct sysfs_class_device *cdev);
 *
 * flag:
 * 	0:	cdev -> valid 
 * 	1:	cdev -> NULL 
 */
int test_sysfs_refresh_classdev_attributes(int flag)
{
	struct dlist *list = NULL;
	struct sysfs_class_device *cdev = NULL;
	char *path = NULL;

	switch (flag) {
	case 0:
		path = val_class_dev_path;
		cdev = sysfs_open_class_device_path(path);
		if (cdev == NULL) {
			dbg_print("%s: failed opening class device at %s\n",
					__FUNCTION__, path);
			return 0;
		}
		break;
	case 1:
		cdev = NULL;
		break;
	default:
		return -1;
	}	
	list = sysfs_refresh_classdev_attributes(cdev);
	
	switch (flag) {
	case 0:
		if (list == NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_attribute_list(list);
			dbg_print("\n");
		}
		break;
	case 1:
		if (list != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
	default:
		break;
	}

	if (cdev != NULL)
		sysfs_close_class_device(cdev);

	return 0;
}

/**
 * extern struct sysfs_attribute *sysfs_get_classdev_attr
 * 	(struct sysfs_class_device *clsdev, const unsigned char *name);
 *
 * flag:
 * 	0:	clsdev -> valid, name -> valid
 * 	1:	clsdev -> valid, name -> invalid
 * 	2:	clsdev -> valid, name -> NULL
 * 	3:	clsdev -> NULL, name -> valid
 * 	4:	clsdev -> NULL, name -> invalid
 * 	5:	clsdev -> NULL, name -> NULL
 */
int test_sysfs_get_classdev_attr(int flag)
{
	struct sysfs_attribute *attr = NULL;
	struct sysfs_class_device *clsdev = NULL;
	char *name = NULL;
	char *path = NULL;

	switch (flag) {
	case 0:
		path = val_class_dev_path;
		clsdev = sysfs_open_class_device_path(path);
		if (clsdev == NULL) {
			dbg_print("%s: failed opening class device at %s\n",
					__FUNCTION__, path);
			return 0;
		}
		name = val_class_dev_attr;
		break;
	case 1:
		path = val_class_dev_path;
		clsdev = sysfs_open_class_device_path(path);
		if (clsdev == NULL) {
			dbg_print("%s: failed opening class device at %s\n",
					__FUNCTION__, path);
			return 0;
		}
		name = inval_name;
		break;
	case 2:
		path = val_class_dev_path;
		clsdev = sysfs_open_class_device_path(path);
		if (clsdev == NULL) {
			dbg_print("%s: failed opening class device at %s\n",
					__FUNCTION__, path);
			return 0;
		}
		name = NULL;
		break;
	case 3:
		clsdev = NULL;
		name = val_class_dev_attr;
		break;
	case 4:
		clsdev = NULL;
		name = inval_name;
		break;
	case 5:
		clsdev = NULL;
		name = NULL;
		break;
	default:
		return -1;
	}
	attr = sysfs_get_classdev_attr(clsdev, name);

	switch (flag) {
	case 0:
		if (attr == NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_attribute(attr);
			dbg_print("\n");
		}
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		if (attr != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
	default:
		break;

	}
	if (clsdev != NULL)
		sysfs_close_class_device(clsdev);

	return 0;
}

/**
 * extern struct sysfs_attribute *sysfs_open_classdev_attr
 * 			(const unsigned char *classname, 
 * 			const unsigned char *dev, 
 * 			const unsigned char *attrib); 
 * flag:
 * 	0:	classname -> valid, dev -> valid, attrib -> valid
 * 	1:	classname -> valid, dev -> valid, attrib -> invalid
 * 	2:	classname -> valid, dev -> valid, attrib -> NULL
 * 	3:	classname -> valid, dev -> invalid, attrib -> valid
 * 	4:	classname -> valid, dev -> invalid, attrib -> invalid
 * 	5:	classname -> valid, dev -> invalid, attrib -> NULL
 * 	6:	classname -> valid, dev -> NULL , attrib -> valid
 * 	7:	classname -> valid, dev -> NULL , attrib -> invalid
 * 	8:	classname -> valid, dev -> NULL , attrib -> NULL
 * 	9:	classname -> invalid, dev -> valid, attrib -> valid
 * 	10:	classname -> invalid, dev -> valid, attrib -> invalid
 * 	11:	classname -> invalid, dev -> valid, attrib -> NULL
 * 	12:	classname -> invalid, dev -> invalid, attrib -> valid
 * 	13:	classname -> invalid, dev -> invalid, attrib -> invalid
 * 	14:	classname -> invalid, dev -> invalid, attrib -> NULL
 * 	15:	classname -> invalid, dev -> NULL , attrib -> valid
 * 	16:	classname -> invalid, dev -> NULL , attrib -> invalid
 * 	17:	classname -> invalid, dev -> NULL , attrib -> NULL
 * 	18:	classname -> NULL, dev -> valid, attrib -> valid
 * 	19:	classname -> NULL, dev -> valid, attrib -> invalid
 * 	20:	classname -> NULL, dev -> valid, attrib -> NULL
 * 	21:	classname -> NULL, dev -> invalid, attrib -> valid
 * 	22:	classname -> NULL, dev -> invalid, attrib -> invalid
 * 	23:	classname -> NULL, dev -> invalid, attrib -> NULL
 * 	24:	classname -> NULL, dev -> NULL , attrib -> valid
 * 	25	classname -> NULL, dev -> NULL , attrib -> invalid
 * 	26:	classname -> NULL, dev -> NULL , attrib -> NULL
 */
int  test_sysfs_open_classdev_attr(int flag)
{
	struct sysfs_attribute *attr = NULL;
	char *classname = NULL;
	char *dev = NULL;
	char *attrib = NULL;

	switch (flag) {
	case 0:
		classname = val_class;
		dev = val_class_dev;
		attrib = val_class_dev_attr;
		break;
	case 1:
		classname = val_class;
		dev = val_class_dev;
		attrib = inval_name;
		break;
	case 2:
		classname = val_class;
		dev = val_class_dev;
		attrib = NULL;
		break;
	case 3:
		classname = val_class;
		dev= inval_name;
		attrib = val_class_dev_attr;
		break;
	case 4:
		classname = val_class;
		dev= inval_name;
		attrib = inval_name;
		break;
	case 5:
		classname = val_class;
		dev= inval_name;
		attrib = NULL;
		break;
	case 6:
		classname = val_class;
		dev = NULL;
		attrib = val_class_dev_attr;
		break;
	case 7:
		classname = val_class;
		dev = NULL;
		attrib = inval_name;
		break;
	case 8:
		classname = val_class;
		dev = NULL;
		attrib = NULL;
		break;
	case 9:
		classname = inval_name;
		dev = val_class_dev;
		attrib = val_class_dev_attr;
		break;
	case 10:
		classname = inval_name;
		dev = val_class_dev;
		attrib = inval_name;
		break;
	case 11:
		classname = inval_name;
		dev = val_class_dev;
		attrib = NULL;
		break;
	case 12:
		classname = inval_name;
		dev= inval_name;
		attrib = val_class_dev_attr;
		break;
	case 13:
		classname = inval_name;
		dev= inval_name;
		attrib = inval_name;
		break;
	case 14:
		classname = inval_name;
		dev= inval_name;
		attrib = NULL;
		break;
	case 15:
		classname = inval_name;
		dev = NULL;
		attrib = val_class_dev_attr;
		break;
	case 16:
		classname = inval_name;
		dev = NULL;
		attrib = inval_name;
		break;
	case 17:
		classname = inval_name;
		dev = NULL;
		attrib = NULL;
		break;
	case 18:
		classname = NULL;
		dev = val_class_dev;
		attrib = val_class_dev_attr;
		break;
	case 19:
		classname = NULL;
		dev = val_class_dev;
		attrib = inval_name;
		break;
	case 20:
		classname = NULL;
		dev = val_class_dev;
		attrib = NULL;
		break;
	case 21:
		classname = NULL;
		dev= inval_name;
		attrib = val_class_dev_attr;
		break;
	case 22:
		classname = NULL;
		dev= inval_name;
		attrib = inval_name;
		break;
	case 23:
		classname = NULL;
		dev= inval_name;
		attrib = NULL;
		break;
	case 24:
		classname = NULL;
		dev = NULL;
		attrib = val_class_dev_attr;
		break;
	case 25:
		classname = NULL;
		dev = NULL;
		attrib = inval_name;
		break;
	case 27:
		classname = NULL;
		dev = NULL;
		attrib = NULL;
		break;
	default:
		return -1;
	}
	attr = sysfs_open_classdev_attr(classname, dev, attrib);

	switch (flag) {
	case 0:
		if (attr == NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_attribute(attr);
			dbg_print("\n");
		}
		break;
	default:
		if (attr != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		break;

	}
	if (attr != NULL)
		sysfs_close_attribute(attr);

	return 0;
}