/* write attribute support for the given device */

#include <stdio.h>

#include "libsysfs.h"

int main(int argc, char *argv[])
{
	/*
	 * need args: device, attribute and value to be changed to
	 * eg: ./fn_name <device> <attribut> <val to change to>
	 */ 
	if (argc != 4) {
		fprintf(stdout, "Need 4 args\n");
		return 1;
	}
/*	if ((sysfs_change_attribute_value(argv[1], argv[2], argv[3])) < 0) {*/
	if ((sysfs_write_device_attr(argv[1], argv[2], argv[3], strlen(argv[3]))) < 0) {
/*	if ((sysfs_write_classdev_attr(argv[1], argv[2], argv[3])) < 0) {*/
/*	if ((sysfs_write_driver_attr(argv[1], argv[2], argv[3])) < 0) {*/
		fprintf(stdout, "Write attribute error\n");
		return 1;
	}
	fprintf(stdout, "Write succeeded\n");
	return 0;
}

