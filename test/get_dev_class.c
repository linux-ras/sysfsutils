/*
 * get_dev_class.c
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
        fprintf(stdout, "Usage: get_dev_class [class_device]\n");
}

int main(int argc, char *argv[])
{
	char *classname = NULL;
	int rc;

	if (argc != 2) {
		print_usage();
		return 1;
	}

	classname = (char *)calloc(1, SYSFS_NAME_LEN);
	rc = sysfs_find_device_class(argv[1], classname, SYSFS_NAME_LEN);
	if (rc == -1) {
		fprintf(stdout, "Device %s not found\n", argv[1]);
		free(classname);
		return 1;
	}
	fprintf(stdout, "Device %s is a member of class %s\n", 
			argv[1], classname);
	free(classname);
	return 0;
}

