EXTRA_DIST = docs include
SUBDIRS = lib cmd

dist-hook:
	rm -rf `find $(distdir)/docs -name CVS`
	rm -rf `find $(distdir)/include -name CVS`

