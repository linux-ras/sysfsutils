/*
 * get_class_dev.c
 *
 * Utility to find the class a given device is on
 *
 * Copyright (C) IBM Corp. 2003
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

void print_usage(void)
{
        fprintf(stdout, "Usage: get_class_dev [class name] [class_device]\n");
}

int main(int argc, char *argv[])
{
	struct sysfs_class_device *cdev = NULL;
	struct sysfs_attribute *attr = NULL;
	struct dlist *attrlist = NULL;

	if (argc != 3) {
		print_usage();
		return 1;
	}

	cdev = sysfs_open_class_device_by_name(argv[1], argv[2]);
	if (cdev == NULL) {
		fprintf(stdout, "Device %s not found\n", argv[2]);
		return 1;
	}
	
	fprintf(stdout, "Class device %s\n", cdev->name);
	
	attrlist = sysfs_get_classdev_attributes(cdev);
	if (attrlist != NULL) {
		dlist_for_each_data(attrlist, attr, struct sysfs_attribute) 
			fprintf(stdout, "\t%s : %s", attr->name, attr->value);
	}
	fprintf(stdout, "\n");
		
	if (cdev->sysdevice)
		fprintf(stdout, "\tDevice : %s\n", cdev->sysdevice->bus_id);
	if (cdev->driver)
		fprintf(stdout, "\tDriver : %s\n", cdev->driver->name);

	sysfs_close_class_device(cdev);
	return 0;
}

