check_PROGRAMS = dlist_test get_device get_driver get_module testlibsysfs
BUILT_SOURCES = test.h
CLEANFILES = test.h
test.h: libsysfs.conf create-test
	$(AM_V_GEN) $(srcdir)/create-test $(srcdir)/libsysfs.conf
get_device_SOURCES = get_device.c
get_driver_SOURCES = get_driver.c
get_module_SOURCES = get_module.c
testlibsysfs_SOURCES = test.c test_attr.c test_bus.c test_class.c \
		       test_device.c test_driver.c test_module.c test_utils.c \
		       testout.c test-defs.h libsysfs.conf create-test
AM_CPPFLAGS = -I$(top_srcdir)/include
LDADD = $(top_builddir)/lib/libsysfs.la
AM_CFLAGS = -Wall -W -Wextra -Wstrict-prototypes $(EXTRA_CFLAGS)

