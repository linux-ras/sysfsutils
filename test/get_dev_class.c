/* Gets class of the supplied device */

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
	char *classname = NULL;
	int rc;

	if (argc != 2) {
		fprintf(stdout, "Need 2 argx\n");
		return 1;
	}

	classname = (char *)calloc(1, SYSFS_NAME_LEN);
	rc = sysfs_find_device_class_name(argv[1], classname, SYSFS_NAME_LEN);
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

