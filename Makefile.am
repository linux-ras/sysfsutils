EXTRA_DIST = docs include
SUBDIRS = lib cmd test
include_HEADERS = include/libsysfs.h include/dlist.h

dist-hook:
	rm -rf `find $(distdir)/docs -name CVS`
	rm -rf `find $(distdir)/include -name CVS`

