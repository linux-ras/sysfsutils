/* write attribute support for the given device */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mntent.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#include "libsysfs.h"

int main(int argc, char *argv[])
{
	char *bus = NULL;
	struct sysfs_device *device = NULL;
	struct sysfs_attribute *attribute = NULL;

	/*
	 * need args: device, attribute and value to be changed to
	 * eg: ./fn_name <device> <attribute> <val to change to>
	 */ 
	if (argc != 4) {
		fprintf(stdout, "Need 4 args\n");
		return 1;
	}

	bus = (char *)calloc(1, SYSFS_NAME_LEN);
	device = sysfs_open_device_by_name(argv[1], bus, SYSFS_NAME_LEN);
	if (device == NULL) {
		fprintf(stdout, "Device %s not found\n", argv[1]);
		free(bus);
		return 1;
	}
	fprintf(stdout, "Device %s is a member of bus %s\n", 
			argv[1], bus);
	fprintf(stdout, "\t\t%s\n", device->bus_id);
	
	attribute = sysfs_get_device_attr(device, argv[2]);
	if (attribute == NULL) {
		fprintf(stdout, "Attribute %s not defined for device %s\n",
				argv[2], argv[1]);
		sysfs_close_device(device);
		free(bus);
		return 1;
	}

	if ((sysfs_write_attribute_value(attribute->path, argv[2])) < 0) {
		fprintf(stdout, "Write attribute failed\n");
		sysfs_close_device(device);
		free(bus);
		return 1;
	}

	fprintf(stdout, "Write succeeded\n");
	sysfs_close_device(device);
	free(bus);
	return 0;
}

