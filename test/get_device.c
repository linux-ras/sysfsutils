/* Gets details of the supplied device */

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
	struct sysfs_driver *driver = NULL;
	struct sysfs_device *device = NULL;
	struct sysfs_attribute *attr = NULL;

	if (argc != 2) {
		fprintf(stdout, "Need 2 args\n");
		return 1;
	}

	bus = (char *)calloc(1, SYSFS_NAME_LEN);
	if ((sysfs_find_device_bus(argv[1], bus, SYSFS_NAME_LEN)) < 0) {
		fprintf(stdout, "Device %s not found\n", argv[1]);
		free (bus);
		return 1;
	}
	fprintf(stdout, "Device %s is a member of bus %s\n", 
			argv[1], bus);
	device = sysfs_open_device_by_id(argv[1], bus, SYSFS_NAME_LEN);
	if (device == NULL) {
		fprintf(stdout, "Device %s not found\n", argv[1]);
		free(bus);
		return 1;
	}
	dlist_for_each_data(device->directory->attributes, attr, 
					struct sysfs_attribute) {
		fprintf(stdout, "\t%s : %s", attr->name, attr->value);
	}
	fprintf(stdout, "\n");
	
	sysfs_close_device(device);
	free(bus);
	return 0;
}

