/* Gets details of the supplied driver */

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

	if (argc != 2) {
		fprintf(stdout, "Need 2 args\n");
		return 1;
	}

	bus = (char *)calloc(1, SYSFS_NAME_LEN);
	if ((sysfs_find_driver_bus(argv[1], bus, SYSFS_NAME_LEN)) < 0) {
		fprintf(stdout, "Driver %s not found\n", argv[1]);
		free(bus);
		return 1;
	}
	fprintf(stdout, "Driver %s is a member of bus %s\n", 
			argv[1], bus);
	driver = sysfs_open_driver_by_name(argv[1], bus, SYSFS_NAME_LEN);
	if (driver == NULL) {
		fprintf(stdout, "Device %s not found\n", argv[1]);
		free(bus);
		return 1;
	}
	if (driver->devices != NULL) {
		fprintf(stdout, "%s is used by:\n", argv[1]);
		dlist_for_each_data(driver->devices, device, struct sysfs_device) 
			fprintf(stdout, "\t\t%s\n", device->bus_id);
	} else 
		fprintf(stdout, "%s is presently not used by any device\n", argv[1]);
	
	sysfs_close_drv(driver);
	free(bus);
	return 0;
}

