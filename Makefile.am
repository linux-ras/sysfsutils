EXTRA_DIST = docs include CREDITS lib/LGPL test/GPL
SUBDIRS = lib cmd test
include_HEADERS = include/libsysfs.h include/dlist.h

dist-hook:
	rm -rf `find $(distdir)/docs -name CVS`
	rm -rf `find $(distdir)/include -name CVS`

