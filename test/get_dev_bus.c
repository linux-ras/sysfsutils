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
	char *busname = NULL;
	int rc;

	if (argc != 2) {
		fprintf(stdout, "Need 2 argx\n");
		return 1;
	}

	busname = (char *)calloc(1, SYSFS_NAME_LEN);
	rc = sysfs_find_device_bus_name(argv[1], busname, SYSFS_NAME_LEN);
	if (rc == -1) {
		fprintf(stdout, "Device %s not found\n", argv[1]);
		free(busname);
		return 1;
	}
	fprintf(stdout, "Device %s is on bus %s\n", argv[1], busname);
	free(busname);
	return 0;
}

