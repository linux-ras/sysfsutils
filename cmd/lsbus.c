/*
 * lsbus.c
 *
 * Utility to list bus devices
 *
 * Copyright (C) IBM Corp. 2003
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License as published by the
 *	Free Software Foundation version 2 of the License.
 * 
 *	This program is distributed in the hope that it will be useful, but
 *	WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *	General Public License for more details.
 * 
 *	You should have received a copy of the GNU General Public License along
 *	with this program; if not, write to the Free Software Foundation, Inc.,
 *	675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
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

/* Command Options */
static int show_options = 0;		/* bitmask of show options */
static unsigned char *attr_to_show = NULL;	/* print value for this attribute*/
static unsigned char *bus_device = NULL;		/* print only this bus device */

#define SHOW_ATTRIBUTES		0x01	/* show attributes command option */
#define SHOW_ATTRIBUTE_VALUE	0x02	/* show an attribute value option */
#define SHOW_DEVICES		0x04	/* show only devices option */
#define SHOW_DRIVERS		0x08	/* show only drivers option */
#define SHOW_ALL_ATTRIB_VALUES	0x10	/* show all attributes with values */
                                                                                
#define SHOW_ALL		0xff

static unsigned char cmd_options[] = "aA:dDhv";

/*
 * binary_files - defines existing sysfs binary files that should be printed
 * in hex.
 */
static unsigned char *binary_files[] = {
	"config",
	"data"
};

static int binfiles = 2;

/**
 * usage: prints utility usage.
 */
void usage(void)
{
	fprintf(stdout, "Usage: lsbus [<options> bus [device]]\n");
	fprintf(stdout, "\t-a\t\t\tShow attributes\n");
	fprintf(stdout, "\t-d\t\t\tShow only devices\n");
	fprintf(stdout, "\t-h\t\t\tShow usage\n");
	fprintf(stdout, "\t-v\t\t\tShow all attributes with values\n");
	fprintf(stdout, "\t-A <attribute_name>\tShow attribute value\n");
	fprintf(stdout, "\t-D\t\t\tShow only drivers\n");
}

/**
 * remove_end_newline: removes newline on the end of an attribute value
 * @value: string to remove newline from
 */
void remove_end_newline(unsigned char *value)
{
	unsigned char *p = value + (strlen(value) - 1);

	if (p != NULL && *p == '\n')
		*p = '\0';
}

/**
 * print_device_children: prints out device subdirs.
 * @sdir: print this directory's subdirectories/children.
 */
void print_device_children(struct sysfs_directory *sdir)
{
	if (sdir != NULL) {
		fprintf(stdout, "\tChildren:\n");
		if (sdir->subdirs != NULL) {
			struct sysfs_directory *cur;

			dlist_for_each_data(sdir->subdirs, cur,
					struct sysfs_directory) {
				fprintf(stdout, "\t\t%s\n", cur->name);
			}
		}	
		if (sdir->links != NULL) {
			struct sysfs_link *curl = NULL;

			dlist_for_each_data(sdir->links, curl,
					struct sysfs_link) {
				fprintf(stdout, "\t\t%s\n", curl->name);
			}
		}
	}
}

/**
 * isbinaryvalue: checks to see if attribute is binary or not.
 * @attr: attribute to check.
 * returns 1 if binary or 0 if not.
 */
int isbinaryvalue(struct sysfs_attribute *attr)
{
	int i;

	if (attr == NULL || attr->value == NULL)
		return 0;

	for (i = 0; i < binfiles; i++) 
		if ((strcmp(attr->name, binary_files[i])) == 0)
			return 1;

	return 0;
}

/**
 * print_attribute_value: prints out single attribute value.
 * @attr: attricute to print.
 */
void print_attribute_value(struct sysfs_attribute *attr)
{
	if (attr == NULL)
		return;
                                                                                
	if (attr->method & SYSFS_METHOD_SHOW) {
		if (isbinaryvalue(attr) != 0) {
			int i;
			fprintf(stdout, "\t: ");
			for (i = 0; i < attr->len; i++) {
				if (!(i %16)) {
					fprintf(stdout, "\n\t\t\t");
				} else if (!(i %8))
					fprintf(stdout, " ");
				fprintf(stdout, "%02x ",
					(unsigned char)attr->value[i]);
			}
			fprintf(stdout, "\n");
                                                                                
		} else if (attr->value != NULL && strlen(attr->value) > 0) {
			remove_end_newline(attr->value);
			fprintf(stdout, "\t: %s\n", attr->value);
		} else
			fprintf(stdout, "\n");
	} else {
		fprintf(stdout, "\t: store method only\n");
	}
}

/**
 * print_attribute: prints out a single attribute
 * @attr: attribute to print.
 */
void print_attribute(struct sysfs_attribute *attr)
{
	if (attr == NULL) 
		return;

	if (show_options & SHOW_ALL_ATTRIB_VALUES) {
		fprintf(stdout, "\t\t%s", attr->name);
		print_attribute_value(attr);

	} else if ((show_options & SHOW_ATTRIBUTES) || ((show_options 
	    & SHOW_ATTRIBUTE_VALUE) && (strcmp(attr->name, attr_to_show) 
	    == 0))) {
		fprintf (stdout, "\t\t%s", attr->name);
		if (show_options & SHOW_ATTRIBUTE_VALUE && attr->value != NULL
		    && (strcmp(attr->name, attr_to_show)) == 0) {
			print_attribute_value(attr);
		} else {
			fprintf(stdout, "\n");
		}
	}
}

/**
 * print_device_attributes: prints out device attributes.
 * @sdir: print this directory's attributes/files.
 */
void print_device_attributes(struct sysfs_directory *sdir)
{
	if (sdir != NULL && sdir->attributes != NULL) {
		struct sysfs_attribute *cur = NULL;;

		fprintf (stdout, "\tAttributes:\n");
		dlist_for_each_data(sdir->attributes, cur,
				struct sysfs_attribute) {
			print_attribute(cur);
		}
	}
}

/**
 * print_device: prints out device information.
 * @device: device to print.
 */
void print_device(struct sysfs_device *device)
{
	if (device != NULL && device->directory != NULL) {
		fprintf (stdout, "  %s %s\n", device->bus_id, device->name);
		if (device->directory->subdirs != NULL
		    || device->directory->links != NULL)
			print_device_children(device->directory);
		if (device->directory->attributes != NULL && (show_options 
		    & (SHOW_ATTRIBUTES | SHOW_ATTRIBUTE_VALUE
		    | SHOW_ALL_ATTRIB_VALUES))) 
			print_device_attributes(device->directory);
		if (device->driver != NULL) 
			fprintf (stdout, "\tDriver: %s\n", 
						device->driver->name);
	}
}

/**
 * print_driver_attributes: prints out driver attributes .
 * @driver: print this driver's attributes.
 */
void print_driver_attributes(struct sysfs_driver *driver)
{
	if (driver != NULL && driver->directory != NULL) {
		struct sysfs_directory *sdir = driver->directory;

		if (sdir->attributes != NULL) {
			struct sysfs_attribute *cur = NULL;

			fprintf (stdout, "\t%s Attributes:\n", driver->name);
			dlist_for_each_data(sdir->attributes, cur, 
					struct sysfs_attribute) {
				print_attribute(cur);
			}
		}
	}
}

/**
 * print_driver_devices: prints out devices under driver.
 * @driver: print devices bound to this driver.
 */
void print_driver_devices(struct sysfs_driver *driver)
{
	if (driver != NULL && driver->directory != NULL) {
		struct sysfs_directory *sdir = driver->directory;

		if (sdir->links != NULL) {
			struct sysfs_link *cur = NULL;

			fprintf (stdout, "\tDevices:\n");
			dlist_for_each_data(sdir->links, cur,
					struct sysfs_link) {
				fprintf(stdout, "\t\t%s\n", cur->name);
			}
		}
	}
}

/**
 * print_driver: prints out driver information.
 * @driver: driver to print.
 */
void print_driver(struct sysfs_driver *driver)
{
	struct sysfs_directory *sdir = NULL;

	if (driver != NULL) {
		fprintf (stdout, "  %s\n", driver->name);
		sdir = driver->directory;
		if (sdir != NULL) {
			if (sdir->links != NULL)
				print_driver_devices(driver);
			if(sdir->attributes != NULL && (show_options
			    & (SHOW_ATTRIBUTES | SHOW_ATTRIBUTE_VALUE
			    | SHOW_ALL_ATTRIB_VALUES)))
				print_driver_attributes(driver);
		}
	}
}

/**
 * print_default_bus: prints out everything on a bus.
 * @busname: bus to print.
 * returns 0 with success or 1 with error.
 */
int print_sysfs_bus(unsigned char *busname)
{
	struct sysfs_bus *bus = NULL;
	struct sysfs_device *curdev = NULL;
	struct sysfs_driver *curdrv = NULL;

	if (busname == NULL) {
		errno = EINVAL;
		return 1;
	}
	bus = sysfs_open_bus(busname);
	if (bus == NULL) {
		fprintf(stderr, "Error opening bus %s\n", busname);
		return 1;
	}

	fprintf(stdout, "Bus: %s\n", busname);
	if (bus->devices != NULL && (show_options & SHOW_DEVICES)) {
		if (bus_device == NULL)
			fprintf(stdout, "Devices:\n");
		dlist_for_each_data(bus->devices, curdev,
				struct sysfs_device) {
			if (bus_device == NULL || (strcmp(bus_device,
			    curdev->bus_id) == 0))
				print_device(curdev);
		}
	}
	if (bus->drivers != NULL && (show_options & SHOW_DRIVERS)) {
		fprintf(stdout, "Drivers:\n");
		dlist_for_each_data(bus->drivers, curdrv, struct sysfs_driver) {
			print_driver(curdrv);
		}
	}
	sysfs_close_bus(bus);
	return 0;
}

/**
 * print_sysfs_buses: prints out supported buses.
 * @buses: list of supported system buses.
 * returns 0 with success or 1 with error.
 */
int print_sysfs_buses(void)
{
	struct sysfs_directory *busdir = NULL, *current = NULL;
	unsigned char buspath[SYSFS_PATH_MAX];
	int ret = 0;

	/* get sysfs mount point */
	if ((sysfs_get_mnt_path(buspath, SYSFS_PATH_MAX) != 0)) {
		perror("sysfs_get_mnt_path");
		fprintf(stderr, "Error getting sysfs mount path - exiting\n");
		exit(1);
	}
	strcat(buspath, SYSFS_BUS_DIR);
	busdir = sysfs_open_directory(buspath);
	if (busdir == NULL) {
		fprintf(stderr, "Error opening sysfs bus directory at %s\n",
			buspath);
		return 1;
	}
	if ((sysfs_read_directory(busdir)) != 0) {
		fprintf(stderr, "Error reading sysfs bus directory at %s\n",
			buspath);
		sysfs_close_directory(busdir);
		return 1;
	}

	fprintf(stdout, "Supported sysfs buses:\n");
	dlist_for_each_data(busdir->subdirs, current, struct sysfs_directory) {
		fprintf(stdout, "\t%s\n", current->name);
	}
	sysfs_close_directory(busdir);
	return ret;
}

/* MAIN */
int main(int argc, char *argv[])
{
	unsigned char *bus_to_print = NULL;
	int retval = 0;
	int opt;
	extern int optind;
	extern char *optarg;

	while((opt = getopt(argc, argv, cmd_options)) != EOF) {
		switch(opt) {
		case 'a':
			show_options |= SHOW_ATTRIBUTES;
			break;
		case 'A':
			if ((strlen(optarg) + 1) > SYSFS_NAME_LEN) {
				fprintf(stderr, 
					"Attribute name %s is too long\n",
					optarg);
				exit(1);
			}
			attr_to_show = optarg;
			show_options |= SHOW_ATTRIBUTE_VALUE;
			break;	
		case 'd':
			show_options |= SHOW_DEVICES;
			break;
		case 'D':
			show_options |= SHOW_DRIVERS;
			break;
		case 'h':
			usage();
			exit(0);
			break;
		case 'v':
			show_options |= SHOW_ALL_ATTRIB_VALUES;
			break;
		default:
			usage();
			exit(1);
		}
	}
	argc -= optind;
	argv += optind;

	switch(argc) {
	case 0:
		break;
	case 1:
		/* get bus to view */
		if ((strlen(*argv)+1) < SYSFS_NAME_LEN) 
			bus_to_print = *argv;
		else
			fprintf(stderr, 
				"Invalid argument: busname too long\n");
		break;
	case 2:
		/* get bus and device to view */
		if ((strlen(argv[0])) < SYSFS_NAME_LEN && (strlen(argv[1]))
	 	    < SYSFS_NAME_LEN) {
			bus_to_print = argv[0];
			bus_device = argv[1];
			show_options |= SHOW_DEVICES;
		} else {
			fprintf(stderr, 
				"Invalid argument: bus or device name too long\n");
			exit(1);
		}
		break;
	default:
		usage();
		exit(1);
	}

	if (bus_to_print == NULL && (show_options & (SHOW_ATTRIBUTES 
	    | SHOW_ATTRIBUTE_VALUE | SHOW_DEVICES | SHOW_DRIVERS
	    | SHOW_ALL_ATTRIB_VALUES))) {
		fprintf(stderr, "Please specify a bus\n");
		usage();
		exit(1);
	}
	/* default is to print devices */
	if (!(show_options & (SHOW_DEVICES | SHOW_DRIVERS)))
		show_options |= SHOW_DEVICES;

	if (bus_to_print != NULL)  
		retval = print_sysfs_bus(bus_to_print);
	else
		retval = print_sysfs_buses();

	exit(retval);
}
