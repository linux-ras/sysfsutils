/*
 * get_class_dev_attr.c
 *
 * Utility to find an attr of a class device
 *
 * Copyright (C) IBM Corp. 2003-2005
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

#include <stdio.h>
#include <stdlib.h>

#include "libsysfs.h"

static void print_usage(void)
{
        fprintf(stdout, "Usage: get_class_dev_attr [class name] [class_device] [attribute]\n");
}

int main(int argc, char *argv[])
{
	struct sysfs_class *cls = NULL;
	struct sysfs_class_device *cdev = NULL;
	struct sysfs_device *dev = NULL;
	struct sysfs_attribute *attr;
	struct dlist *list;

	if (argc != 4) {
		print_usage();
		return 1;
	}

	cls = sysfs_open_class(argv[1]);
	if (!cls) {
		fprintf(stdout, "Error opening class\n");
		return 1;
	}

	list = sysfs_get_class_devices(cls);
	if (!list)
		return 1;
	dlist_for_each_data(list, cdev, struct sysfs_class_device) {
	if (!cdev) {
		fprintf(stdout, "Device \"%s\" not found\n", argv[2]);
		return 1;
	}

	fprintf(stdout, "Class device \"%s\"\n", cdev->name);
	fprintf(stdout, "Class \"%s\"\n", cdev->classname);

	attr = sysfs_get_classdev_attr(cdev, argv[3]);
	if (attr)
		fprintf(stdout, "\t%-20s : %s", attr->name, attr->value);
	fprintf(stdout, "\n");

	dev = sysfs_get_classdev_device(cdev);
	if (dev)
		fprintf(stdout, "device is %s on bus %s using driver %s\n",
				dev->name, dev->bus, dev->driver_name);
	else
		fprintf(stdout, "no device found\n");
	}
	sysfs_close_class(cls);
	return 0;
}

