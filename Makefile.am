EXTRA_DIST = docs include
SUBDIRS = lib cmd test

dist-hook:
	rm -rf `find $(distdir)/docs -name CVS`
	rm -rf `find $(distdir)/include -name CVS`

