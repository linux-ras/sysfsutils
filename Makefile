# sysutils Makefile
#
# Copyright (c) International Business Machines Corp., 2003
#

all:
	$(MAKE) -C lib
	$(MAKE) -C cmd

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C cmd clean
