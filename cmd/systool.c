/*
 * systool.c
 *
 * Sysfs utility to list buses, classes, and devices
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
static unsigned char *attribute_to_show = NULL;	/* show value for this attribute */
static unsigned char *device_to_show = NULL;	/* show only this bus device */

#define SHOW_ATTRIBUTES		0x01	/* show attributes command option */
#define SHOW_ATTRIBUTE_VALUE	0x02	/* show an attribute value option */
#define SHOW_DEVICES		0x04	/* show only devices option */
#define SHOW_DRIVERS		0x08	/* show only drivers option */
#define SHOW_ALL_ATTRIB_VALUES	0x10	/* show all attributes with values */
                                                                                
#define SHOW_ALL		0xff

static unsigned char cmd_options[] = "aA:b:c:dDhr:v";

/*
 * binary_files - defines existing sysfs binary files. These files will be
 * printed in hex.
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
	fprintf(stdout, "Usage: systool [<options> [device]]\n");
	fprintf(stdout, "\t-a\t\t\tShow attributes\n");
	fprintf(stdout, "\t-b <bus_name>\t\tShow a specific bus\n");
	fprintf(stdout, "\t-c <class_name>\t\tShow a specific class\n");
	fprintf(stdout, "\t-d\t\t\tShow only devices\n");
	fprintf(stdout, "\t-h\t\t\tShow usage\n");
	fprintf(stdout, 
		"\t-r <root_device>\tShow a specific root device tree\n");
	fprintf(stdout, "\t-v\t\t\tShow all attributes with values\n");
	fprintf(stdout, "\t-A <attribute_name>\tShow attribute value\n");
	fprintf(stdout, "\t-D\t\t\tShow only drivers\n");
}

/**
 * indent: called before printing a line, it adds indent to the line up to
 *	level passed in.
 * @level: number of spaces to indent.
 */
void indent(int level)
{
	int i;

	for (i = 0; i <= level; i++)
		fprintf(stdout, " ");
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
 * show_device_children: prints out device subdirs.
 * @sdir: print this directory's subdirectories/children.
 */
void show_device_children(struct sysfs_directory *sdir, int level)
{
	if (sdir != NULL) {
		indent(level);
		fprintf(stdout, "Children:\n");
		if (sdir->subdirs != NULL) {
			struct sysfs_directory *cur;
		
			dlist_for_each_data(sdir->subdirs, cur, 
					struct sysfs_directory) {
				indent(level+4);
				fprintf(stdout, "%s\n", cur->name);
			}
		}
		if (sdir->links != NULL) {
			struct sysfs_link *curl = NULL;
			
			dlist_for_each_data(sdir->links, curl,
					struct sysfs_link) {
				indent(level+4);
				fprintf(stdout, "%s\n", curl->name);
			}
		}
	}
}

/**
 * isbinaryvalue: checks to see if attribute is binary or not.
 * @attr: attribute to check.
 * returns 1 if binary, 0 if not.
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
 * show_attribute_value: prints out single attribute value.
 * @attr: attricute to print.
 */
void show_attribute_value(struct sysfs_attribute *attr, int level)
{
	if (attr == NULL)
		return;

	if (attr->method & SYSFS_METHOD_SHOW) {
		if (isbinaryvalue(attr) != 0) {
			int i;
			fprintf(stdout, "\t: ");
			for (i = 0; i < attr->len; i++) {
				if (!(i %16)) {
					fprintf(stdout, "\n");
					indent(level+8);
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
 * show_attribute: prints out a single attribute
 * @attr: attribute to print.
 */
void show_attribute(struct sysfs_attribute *attr, int level)
{
	if (attr == NULL) 
		return;

	if (show_options & SHOW_ALL_ATTRIB_VALUES) {
		indent(level);
		fprintf(stdout, "%s", attr->name);
		show_attribute_value(attr, level);

	} else if ((show_options & SHOW_ATTRIBUTES) || ((show_options 
	    & SHOW_ATTRIBUTE_VALUE) && (strcmp(attr->name, attribute_to_show) 
	    == 0))) {
		indent(level);
		fprintf (stdout, "%s", attr->name);
		if (show_options & SHOW_ATTRIBUTE_VALUE && attr->value 
		    != NULL && (strcmp(attr->name, attribute_to_show)) == 0) 
			show_attribute_value(attr, level);

		else 
			fprintf(stdout, "\n");
	}
}

/**
 * show_attributes: prints out a directory's attributes.
 * @sdir: print this directory's attributes/files.
 */
void show_attributes(struct sysfs_directory *sdir, int level)
{
	if (sdir != NULL && sdir->attributes != NULL) {
		struct sysfs_attribute *cur = NULL;

		indent(level);
		fprintf (stdout, "Attributes:\n");
		dlist_for_each_data(sdir->attributes, cur, 
				struct sysfs_attribute) {
			show_attribute(cur, (level+4));
		}
	}
}

/**
 * show_device: prints out device information.
 * @device: device to print.
 */
void show_device(struct sysfs_device *device, int level)
{
	if (device != NULL && device->directory != NULL) {
		indent(level);
		fprintf (stdout, "%s %s\n", device->bus_id, device->name);
		if (device->directory->subdirs != NULL
		    || device->directory->links != NULL)
			show_device_children(device->directory, (level+4));
		if (device->directory->attributes != NULL && (show_options 
		    & (SHOW_ATTRIBUTES | SHOW_ATTRIBUTE_VALUE
		    | SHOW_ALL_ATTRIB_VALUES))) 
			show_attributes(device->directory, (level+4));
		if (device->driver != NULL) {
			indent(level+4);
			fprintf (stdout, "Driver: %s\n", 
						device->driver->name);
		}
	}
}

/**
 * show_root_device: prints out sys/devices device information.
 * @device: device to print.
 */
void show_root_device(struct sysfs_device *device, int level)
{
	if (device != NULL && device->directory != NULL) {
		indent(level);
		fprintf (stdout, "%s %s\n", device->bus_id, device->name);
		if (device->directory->attributes != NULL && (show_options 
		    & (SHOW_ATTRIBUTES | SHOW_ATTRIBUTE_VALUE
		    | SHOW_ALL_ATTRIB_VALUES)))
			show_attributes(device->directory, (level+4));
	}
}

/**
 * show_driver_attributes: prints out driver attributes .
 * @driver: print this driver's attributes.
 */
void show_driver_attributes(struct sysfs_driver *driver, int level)
{
	if (driver != NULL && driver->directory != NULL) {
		struct sysfs_directory *sdir = driver->directory;

		if (sdir->attributes != NULL) {
			struct sysfs_attribute *cur = NULL;

			indent(level);
			fprintf (stdout, "%s Attributes:\n", driver->name);
			dlist_for_each_data(sdir->attributes, cur,
					struct sysfs_attribute) {
				show_attribute(cur, (level+4));
			}
		}
	}
}

/**
 * show_driver_devices: prints out devices under driver.
 * @driver: print devices bound to this driver.
 */
void show_driver_devices(struct sysfs_driver *driver, int level)
{
	if (driver != NULL && driver->directory != NULL) {
		struct sysfs_directory *sdir = driver->directory;

		if (sdir->links != NULL) {
			struct sysfs_link *cur = NULL;

			indent(level);
			fprintf (stdout, "Devices:\n");
			dlist_for_each_data(sdir->links, cur,
					struct sysfs_link) {
				indent(level+4);
				fprintf (stdout, "%s\n", cur->name);
			}
		}
	}
}

/**
 * show_driver: prints out driver information.
 * @driver: driver to print.
 */
void show_driver(struct sysfs_driver *driver, int level)
{
	struct sysfs_directory *sdir = NULL;

	if (driver != NULL) {
		indent(level);
		fprintf (stdout, "%s\n", driver->name);
		sdir = driver->directory;
		if (sdir != NULL) {
			if (sdir->links != NULL)
				show_driver_devices(driver, (level+4));
			if(sdir->attributes != NULL && (show_options
			    & (SHOW_ATTRIBUTES | SHOW_ATTRIBUTE_VALUE
			    | SHOW_ALL_ATTRIB_VALUES)))
				show_driver_attributes(driver, (level+4));
		}
	}
}

/**
 * show_device_tree: prints out device tree.
 * @root: root device
 */
void show_device_tree(struct sysfs_device *root, int level)
{
	if (root != NULL) {
		struct sysfs_device *cur = NULL;

		if (device_to_show == NULL || (strcmp(device_to_show,
			    root->bus_id) == 0)) {
			show_root_device(root, level);
		}
		if (root->children != NULL) {
			dlist_for_each_data(root->children, cur, 
					struct sysfs_device) {
				show_device_tree(cur, (level+6));
			}
		}
	}
}

/**
 * show_sysfs_bus: prints out everything on a bus.
 * @busname: bus to print.
 * returns 0 with success or 1 with error.
 */
int show_sysfs_bus(unsigned char *busname)
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
		if (device_to_show == NULL)
			fprintf(stdout, "Devices:\n");
		dlist_for_each_data(bus->devices, curdev, struct sysfs_device) {
			if (device_to_show == NULL || (strcmp(device_to_show,
			    curdev->bus_id) == 0))
				show_device(curdev, 2);
		}
	}
	if (bus->drivers != NULL && (show_options & SHOW_DRIVERS)) {
		fprintf(stdout, "Drivers:\n");
		dlist_for_each_data(bus->drivers, curdrv, struct sysfs_driver) {
			show_driver(curdrv, 2);
		}
	}
	sysfs_close_bus(bus);
	return 0;
}

/**
 * show_class_device: prints out class device.
 * @dev: class device to print.
 */
void show_class_device(struct sysfs_class_device *dev, int level)
{
	struct sysfs_directory *cur = NULL;

	if (dev != NULL) {
		indent(level);
		fprintf(stdout, "%s\n", dev->name);
		if (dev->directory != NULL && (show_options
		    & (SHOW_ATTRIBUTES | SHOW_ATTRIBUTE_VALUE
		    | SHOW_ALL_ATTRIB_VALUES))) {
			show_attributes(dev->directory, (level+4));
			dlist_for_each_data(dev->directory->subdirs, cur,
					struct sysfs_directory) {
				indent(level+4);
				fprintf(stdout, "%s\n", cur->name);
				show_attributes(cur, (level+4));
			}
		}
		if (dev->sysdevice != NULL && (show_options & SHOW_DEVICES))
			show_device(dev->sysdevice, (level+4));
		if (dev->driver != NULL && (show_options & SHOW_DRIVERS))
			show_driver(dev->driver, (level+4));
	}
}

/**
 * show_sysfs_class: prints out sysfs class and all its devices.
 * @classname: class to print.
 * returns 0 with success and 1 with error.
 */
int show_sysfs_class(unsigned char *classname)
{
	struct sysfs_class *cls = NULL;
	struct sysfs_class_device *cur = NULL;

	if (classname == NULL) {
		errno = EINVAL;
		return 1;
	}
	cls = sysfs_open_class(classname);
	if (cls == NULL) {
		fprintf(stderr, "Error opening class %s\n", classname);
		return 1;
	}
	fprintf(stdout, "Class: %s\n", classname);
	if (cls->devices != NULL) {
		if (device_to_show == NULL)
			fprintf(stdout, "Class Devices:\n");
		dlist_for_each_data(cls->devices, cur, 
				struct sysfs_class_device) {
			if (device_to_show == NULL || (strcmp(device_to_show,
			    cur->name) == 0))
				show_class_device(cur, 2);
		}
	}

	sysfs_close_class(cls);
	return 0;
}

/**
 * show_sysfs_root: prints out sysfs root device tree 
 * @rootname: device root to print.
 * returns 0 with success and 1 with error.
 */
int show_sysfs_root(unsigned char *rootname)
{
	struct sysfs_device *root = NULL;
	unsigned char path[SYSFS_PATH_MAX];

	if (rootname == NULL) {
		errno = EINVAL;
		return 1;
	}
	if (sysfs_get_mnt_path(path, SYSFS_PATH_MAX) != 0) {
		perror("sysfs_get_mnt_path");
		fprintf(stderr, "Error getting sysfs mount point\n");
		exit(1);
	}

	strcat(path, SYSFS_DEVICES_DIR);
	strcat(path, "/");
	strcat(path, rootname);
	root = sysfs_open_device_tree(path);
	if (root == NULL) {
		fprintf(stderr, "Error opening root device %s\n", rootname);
		return 1;
	}
	fprintf(stdout, "Root Device Tree: %s\n", rootname);
	show_device_tree(root, 2);
	sysfs_close_device_tree(root);

	return 0;
}

/**
 * show_subdirectories: prints all subdirectory names at path
 * @path: sysfs path where subdirs are.
 * returns 0 with success or 1 with error.
 */
int show_subdirectories(unsigned char *path, int level)
{
	struct sysfs_directory *dir = NULL, *current = NULL;
	int ret = 0;

	dir = sysfs_open_directory(path);
	if (dir == NULL) {
		fprintf(stderr, "Error opening sysfs directory at %s\n", path);
		return 1;
	}
	if ((sysfs_read_directory(dir)) != 0) {
		fprintf(stderr, "Error reading sysfs directory at %s\n", path);
		sysfs_close_directory(dir);
		return 1;
	}

	if (dir->subdirs != NULL) {
		dlist_for_each_data(dir->subdirs, current, 
				struct sysfs_directory) {
			indent(level);
			fprintf(stdout, "%s\n", current->name);
		}
	}
	sysfs_close_directory(dir);
	return ret;
}

/**
 * show_default_info: prints current buses, classes, and root devices
 *	supported by sysfs.
 * returns 0 with success or 1 with error.
 */
int show_default_info(void)
{
	unsigned char sysfs_root[SYSFS_PATH_MAX];
	unsigned char path_to_print[SYSFS_PATH_MAX];
	int retval = 0;

	/* get sysfs mount point */
	if (sysfs_get_mnt_path(sysfs_root, SYSFS_PATH_MAX) != 0) {
		perror("sysfs_get_mnt_path");
		fprintf(stderr, "Error getting sysfs mount point\n");
		exit(1);
	}
	strcpy(path_to_print, sysfs_root);

	/* print supported buses */
	strcat(path_to_print, SYSFS_BUS_DIR);
	fprintf(stdout, "Supported sysfs buses:\n");
	retval = show_subdirectories(path_to_print, 4);

	if (retval == 0) {
		/* print supported classes */
		strcpy(path_to_print, sysfs_root);
		strcat(path_to_print, SYSFS_CLASS_DIR);
		fprintf(stdout, "Supported sysfs classes:\n");
		retval = show_subdirectories(path_to_print, 4);
	}

	if (retval == 0) {
		/* print supported root devices */
		strcpy(path_to_print, sysfs_root);
		strcat(path_to_print, SYSFS_DEVICES_DIR);
		fprintf(stdout, "Supported sysfs root devices:\n");
		retval = show_subdirectories(path_to_print, 4);
	}

	return retval;
}

/* MAIN */
int main(int argc, char *argv[])
{
	unsigned char *show_bus = NULL;
	unsigned char *show_class = NULL;
	unsigned char *show_root = NULL;
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
			attribute_to_show = optarg;
			show_options |= SHOW_ATTRIBUTE_VALUE;
			break;	
		case 'b':
			show_bus = optarg;	
			break;
		case 'c':
			show_class = optarg;	
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
		case 'r':
			show_root = optarg;
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
		if ((strlen(*argv)) < SYSFS_NAME_LEN) {
			device_to_show = *argv;
			show_options |= SHOW_DEVICES;
		} else {
			fprintf(stderr, 
				"Invalid argument - device name too long\n");
			exit(1);
		}
		break;
	default:
		usage();
		exit(1);
	}

	if ((show_bus == NULL && show_class == NULL && show_root == NULL) 
	    && (show_options & (SHOW_ATTRIBUTES | SHOW_ATTRIBUTE_VALUE 
	    | SHOW_DEVICES | SHOW_DRIVERS | SHOW_ALL_ATTRIB_VALUES))) {
		fprintf(stderr, 
			"Please specify a bus, class, or root device\n");
		usage();
		exit(1);
	}
	/* default is to print devices */
	if (!(show_options & (SHOW_DEVICES | SHOW_DRIVERS)))
		show_options |= SHOW_DEVICES;

	if (show_bus != NULL)  
		retval = show_sysfs_bus(show_bus);
	if (show_class != NULL)
		retval = show_sysfs_class(show_class);
	if (show_root != NULL)
		retval = show_sysfs_root(show_root);

	if (show_bus == NULL && show_class == NULL && show_root == NULL) 
		retval = show_default_info();

	exit(retval);
}
