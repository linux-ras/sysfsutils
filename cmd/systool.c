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
#include "names.h"

/* Command Options */
static int show_options = 0;		/* bitmask of show options */
static unsigned char *attribute_to_show = NULL;	/* show value for this attribute */
static unsigned char *device_to_show = NULL;	/* show only this bus device */
struct pci_access *pacc = NULL;
unsigned char *show_bus = NULL;

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

static unsigned int get_pciconfig_word(int offset, unsigned char *buf)
{
        unsigned short val = (unsigned char)buf[offset] |
	                ((unsigned char)buf[offset+1] << 8);
        return val;
}

static unsigned char get_pciconfig_byte(int offset, unsigned char *buf)
{
        return((unsigned char)buf[offset]);
}


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
 * @children: dlist of child devices.
 */
void show_device_children(struct dlist *children, int level)
{
	struct sysfs_device *child = NULL;
	
	if (children != NULL) {
		indent(level);
		fprintf(stdout, "Children:\n");
		dlist_for_each_data(children, child, 
				struct sysfs_device) {
			indent(level+4);
			fprintf(stdout, "%s\n", child->bus_id);
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
 * show_attributes: prints out a list of attributes.
 * @attributes: print this dlist of attributes/files.
 */
void show_attributes(struct dlist *attributes, int level)
{
	if (attributes != NULL) {
		struct sysfs_attribute *cur = NULL;
		
		indent(level);
		fprintf (stdout, "Attributes:\n");
		dlist_for_each_data(attributes, cur, 
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
	struct dlist *attributes = NULL;
        unsigned int vendor_id, device_id;
        unsigned char buf[128], value[256], path[SYSFS_PATH_MAX];
	
	if (device != NULL) {
		indent(level);
		if (show_bus != NULL && (!(strcmp(show_bus, "pci")))) {
			fprintf(stdout, "%s: ", device->bus_id);
			memset(path, 0, SYSFS_PATH_MAX);
			memset(value, 0, SYSFS_PATH_MAX);
			strcpy(path, device->path);
			strcat(path, "/config");
			if ((sysfs_read_attribute_value(path, 
							value, 256)) == 0) {
				vendor_id = get_pciconfig_word
						(PCI_VENDOR_ID, value);
				device_id = get_pciconfig_word
						(PCI_DEVICE_ID, value);
				fprintf(stdout, "%s\n",
					pci_lookup_name(pacc, buf, 128,
					PCI_LOOKUP_VENDOR | PCI_LOOKUP_DEVICE,
						vendor_id, device_id, 0, 0));
			} else 
				fprintf(stdout, "\n");
		} else 
			fprintf(stdout, "%s\n", device->bus_id);

		if (show_options & (SHOW_ATTRIBUTES | SHOW_ATTRIBUTE_VALUE |
					SHOW_ALL_ATTRIB_VALUES)) {
			attributes = sysfs_get_device_attributes(device);
			if (attributes != NULL) 
				show_attributes(attributes, (level+4));
		}
		if (device->children != NULL)
			show_device_children(device->children, (level+4));
		if (isalnum(device->driver_name[0])) {
			indent(level+6);
			fprintf (stdout, "Driver: %s\n", 
						device->driver_name);
		} 
	}
}

/**
 * show_root_device: prints out sys/devices device information.
 * @device: device to print.
 */
void show_root_device(struct sysfs_device *device, int level)
{
	if (device != NULL) {
		indent(level);
		fprintf (stdout, "%s\n", device->bus_id);
		if (show_options & (SHOW_ATTRIBUTES | SHOW_ATTRIBUTE_VALUE
		    | SHOW_ALL_ATTRIB_VALUES)) {
			struct dlist *attributes = NULL;
			attributes = sysfs_get_device_attributes(device);
			if (attributes != NULL)
				show_attributes(attributes, (level+4));
		}
	}
}

/**
 * show_driver_attributes: prints out driver attributes .
 * @driver: print this driver's attributes.
 */
void show_driver_attributes(struct sysfs_driver *driver, int level)
{
	if (driver != NULL) {
		struct dlist *attributes = NULL;
	
		attributes = sysfs_get_driver_attributes(driver);
		if (attributes != NULL) {
			struct sysfs_attribute *cur = NULL;

			indent(level);
			fprintf (stdout, "%s Attributes:\n", driver->name);
			dlist_for_each_data(attributes, cur,
					struct sysfs_attribute) {
				show_attribute(cur, (level+4));
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
	struct dlist *devlist = NULL;
	
	if (driver != NULL) {
		indent(level);
		fprintf (stdout, "%s\n", driver->name);
		devlist = sysfs_get_driver_devices(driver);
		if (devlist != NULL) {
			struct sysfs_device *cur = NULL;
			
			indent(level+4);
			fprintf(stdout, "Devices:\n");
			dlist_for_each_data(devlist, cur, 
					struct sysfs_device) {
				indent(level+8);
				fprintf(stdout, "%s\n", cur->bus_id);
			}
		}
		if (show_options & (SHOW_ATTRIBUTES | SHOW_ATTRIBUTE_VALUE
			    | SHOW_ALL_ATTRIB_VALUES))
			show_driver_attributes(driver, (level+4));
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
	struct dlist *devlist = NULL;
	struct dlist *drvlist = NULL;

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
	if (show_options & SHOW_DEVICES) {
		devlist = sysfs_get_bus_devices(bus);
		if (devlist != NULL) {
			if (device_to_show == NULL)
				fprintf(stdout, "Devices:\n");
			dlist_for_each_data(bus->devices, curdev, 
						struct sysfs_device) {
				if (device_to_show == NULL || 
						(strcmp(device_to_show,
							curdev->bus_id) == 0)) 
					show_device(curdev, 2);
			}
		}
	}
	if (show_options & SHOW_DRIVERS) {
		drvlist = sysfs_get_bus_drivers(bus);
		if (drvlist != NULL) {
			fprintf(stdout, "Drivers:\n");
			dlist_for_each_data(bus->drivers, curdrv, 
					struct sysfs_driver) {
				show_driver(curdrv, 2);
			}
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
	struct dlist *attributes = NULL;
	struct sysfs_device *device = NULL;
	struct sysfs_driver *driver = NULL;
	
	if (dev != NULL) {
		indent(level);
		fprintf(stdout, "%s\n", dev->name);
		if (show_options & (SHOW_ATTRIBUTES | SHOW_ATTRIBUTE_VALUE
		    | SHOW_ALL_ATTRIB_VALUES)) {
			attributes = sysfs_get_classdev_attributes(dev);
			if (attributes != NULL)
				show_attributes(attributes, (level+4));
		}
		if (show_options & (SHOW_DEVICES | SHOW_ALL_ATTRIB_VALUES)) {
			device = sysfs_get_classdev_device(dev);
			if (device != NULL)
				show_device(device, (level+4));
		}
		if (show_options & (SHOW_DRIVERS | SHOW_ALL_ATTRIB_VALUES)) {
			driver = sysfs_get_classdev_driver(dev);
			if (driver != NULL)
				show_driver(driver, (level+4));
		}
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
	struct dlist *clsdevlist = NULL;

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
	clsdevlist = sysfs_get_class_devices(cls);
	if (clsdevlist != NULL) {
		if (device_to_show == NULL)
			fprintf(stdout, "Class Devices:\n");
		dlist_for_each_data(clsdevlist, cur, 
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
	struct sysfs_root_device *root = NULL;
	struct sysfs_device *device = NULL;
	struct dlist *devlist = NULL;
	unsigned char path[SYSFS_PATH_MAX];

	if (rootname == NULL) {
		errno = EINVAL;
		return 1;
	}
	root = sysfs_open_root_device(rootname);
	if (root == NULL) {
		fprintf(stderr, "Error opening root device %s\n", rootname);
		return 1;
	}

	devlist = sysfs_get_root_devices(root);
	if (devlist != NULL) {
		fprintf(stdout, "Root Device Tree: %s\n", rootname);
		
		if (devlist != NULL) {
			dlist_for_each_data(devlist, device, 
						struct sysfs_device) {
				show_device_tree(device, 2);
			}
		}
	}
	sysfs_close_root_device(root);
	
	return 0;
}

/**
 * show_default_info: prints current buses, classes, and root devices
 *	supported by sysfs.
 * returns 0 with success or 1 with error.
 */
int show_default_info(void)
{
	unsigned char subsys[SYSFS_NAME_LEN];
	struct dlist *list = NULL;
	unsigned char *cur = NULL;
	int retval = 0, i;

	strcpy(subsys, SYSFS_BUS_DIR);
	list = sysfs_open_subsystem_list(subsys);
	if (list != NULL) {
		fprintf(stdout, "Supported sysfs buses:\n");
		dlist_for_each_data(list, cur, char)
			fprintf(stdout, "\t%s\n", cur);
	}
	sysfs_close_list(list);

	strcpy(subsys, SYSFS_CLASS_DIR);
	list = sysfs_open_subsystem_list(subsys);
	if (list != NULL) {
		fprintf(stdout, "Supported sysfs classes:\n");
		dlist_for_each_data(list, cur, char)
			fprintf(stdout, "\t%s\n", cur);
	}
	sysfs_close_list(list);

	strcpy(subsys, SYSFS_DEVICES_DIR);
	list = sysfs_open_subsystem_list(subsys);
	if (list != NULL) {
		fprintf(stdout, "Supported sysfs devices:\n");
		dlist_for_each_data(list, cur, char)
			fprintf(stdout, "\t%s\n", cur);
	}
	sysfs_close_list(list);
			
	return retval;
}
	
/* MAIN */
int main(int argc, char *argv[])
{
/*	unsigned char *show_bus = NULL;*/
	unsigned char *show_class = NULL;
	unsigned char *show_root = NULL;
	int retval = 0;
	int opt;
	extern int optind;
	extern char *optarg;
        unsigned char *pci_id_file = "/usr/local/share/pci.ids";
	
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

	if ((show_bus == NULL && show_class == NULL && show_root == NULL) && 
			(show_options & (SHOW_ATTRIBUTES | 
				SHOW_ATTRIBUTE_VALUE | SHOW_DEVICES | 
				SHOW_DRIVERS | SHOW_ALL_ATTRIB_VALUES))) {
		fprintf(stderr, 
			"Please specify a bus, class, or root device\n");
		usage();
		exit(1);
	}
	/* default is to print devices */
	if (!(show_options & (SHOW_DEVICES | SHOW_DRIVERS)))
		show_options |= SHOW_DEVICES;

	if (show_bus != NULL) {
		if (!(strcmp(show_bus, "pci"))) {
			pacc = (struct pci_access *)
				calloc(1, sizeof(struct pci_access));
			pacc->pci_id_file_name = pci_id_file;
			pacc->numeric_ids = 0;
		}
		retval = show_sysfs_bus(show_bus);
	}
	if (show_class != NULL)
		retval = show_sysfs_class(show_class);
	if (show_root != NULL)
		retval = show_sysfs_root(show_root);

	if (show_bus == NULL && show_class == NULL && show_root == NULL) 
		retval = show_default_info();

	if (show_bus != NULL)
		if (!(strcmp(show_bus, "pci")))
			pci_free_name_list(pacc);
	exit(retval);
}
