# Process this file with autoconf to produce a configure script.
AC_INIT(sysfsutils, 0_2_0, linux-diag-devel@lists.sourceforge.net)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([lib/sysfs_bus.c])
AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lsysfs':
AC_CHECK_LIB([sysfs], [sysfs_read_directory])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h mntent.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_LSTAT
AC_FUNC_STAT
AC_CHECK_FUNCS([getmntent getpagesize isascii memset strrchr])

AC_CONFIG_FILES([Makefile
                 cmd/Makefile
                 lib/Makefile
		 test/Makefile])
AC_OUTPUT
