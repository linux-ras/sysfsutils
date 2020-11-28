bin_PROGRAMS = systool
systool_SOURCES = systool.c names.c names.h
LDADD = $(top_builddir)/lib/libsysfs.la
EXTRA_CFLAGS = @EXTRA_CFLAGS@
AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = -Wall -W -Wextra -Wstrict-prototypes $(EXTRA_CFLAGS)

