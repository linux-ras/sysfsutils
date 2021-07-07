EXTRA_DIST = libsysfs.pc.in libsysfs.map

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libsysfs.pc

lib_LTLIBRARIES = libsysfs.la
libsysfs_la_SOURCES = sysfs_utils.c sysfs_attr.c sysfs_class.c dlist.c \
      sysfs_device.c sysfs_driver.c sysfs_bus.c sysfs_module.c sysfs.h
libsysfs_la_CPPFLAGS = -I$(top_srcdir)/include
libsysfs_la_LDFLAGS = -version-info 2:1:0
if HAVE_LINKER_VERSION_SCRIPT
libsysfs_la_LDFLAGS += -Wl,--version-script=$(srcdir)/libsysfs.map
EXTRA_libsysfs_la_DEPENDENCIES = libsysfs.map
endif
libsysfs_la_CFLAGS = -Wall -W -Wextra -Wstrict-prototypes $(EXTRA_CFLAGS)
