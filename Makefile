# sysutils Makefile
#
# Copyright (C) IBM Corp. 2003
#

all:
	$(MAKE) -C lib
	$(MAKE) -C cmd

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C cmd clean
