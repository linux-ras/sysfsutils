/* gets list of devices on a given bus */

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
	struct dlist *name = NULL;
	unsigned char path[SYSFS_PATH_MAX];
	unsigned char *cur = NULL;

	if (argc != 2) {
		fprintf(stdout, "Require 2 args: bus\n");
		return 1;
	}

	memset(path, 0, SYSFS_PATH_MAX);
	strcpy(path, "/bus/");
	strcpy(path, argv[1]);
	strcat(path, "/devices");


	name = sysfs_open_subsystem_list(path);
	dlist_for_each_data(name, cur, char) {
		fprintf(stdout, "\t%s",	cur);
	}

	sysfs_close_list(name);

	return 0;
}

