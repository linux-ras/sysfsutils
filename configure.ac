# Process this file with autoconf to produce a configure script.
AC_INIT([sysfsutils], [2.1.1], [https://github.com/linux-ras/sysfsutils/issues])
AC_CONFIG_SRCDIR([cmd/systool.c])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.11 foreign])
AM_SILENT_RULES([yes])

# Checks for KLIBC support (should be before LT_INIT and AC_PROG_CC)
AC_CHECK_KLIBC

if test "X$KLIBC" = Xyes; then
	# Should disable/enable building of shared libraries ... I am open to better
	# suggestions though (AC_DISABLE_SHARED and co do not do the right thing in
	# a situation like this that its runtime dependent) ...
	enable_shared="no"
	EXTRA_CFLAGS=
else
	# These ones make KLIBC compiles very loud
	EXTRA_CFLAGS="-Wshadow -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs"
fi
AM_CONDITIONAL([KLIBC], [test "X$KLIBC" = Xyes])
AC_SUBST([EXTRA_CFLAGS])

AC_ARG_WITH([pci.ids],
	[AS_HELP_STRING([--with-pci.ids=PATHNAME],
		[specify the pci.ids database PATHNAME])],
	[PCI_IDS_PATHNAME=$with_pci_ids],
	[PCI_IDS_PATHNAME="/usr/share/misc/pci.ids"])
AC_DEFINE_UNQUOTED([PCI_IDS_PATHNAME], ["$PCI_IDS_PATHNAME"],
	[pci.ids database pathname])

# Checks for library support.
LT_INIT
AC_CACHE_CHECK([for --version-script linker flag], [sysfs_cv_version_script], [
	echo "{ global: symbol; local: *; };" >conftest.map
	save_LDFLAGS=$LDFLAGS
	LDFLAGS="$LDFLAGS -Wl,--version-script=conftest.map"
	AC_LINK_IFELSE([
		AC_LANG_PROGRAM([], [])
	], [
		sysfs_cv_version_script=yes
	], [
		sysfs_cv_version_script=no
	])
	LDFLAGS="$save_LDFLAGS"
	rm -f conftest.map
])
AM_CONDITIONAL([HAVE_LINKER_VERSION_SCRIPT],
	[test "x$sysfs_cv_version_script" = "xyes"])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for header files.
AC_HEADER_DIRENT
AC_CHECK_HEADERS([fcntl.h malloc.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_LSTAT
AC_FUNC_MALLOC
AC_FUNC_STAT
AC_CHECK_FUNCS([bzero isascii memset strchr strerror strrchr strstr strtol])

AC_CONFIG_FILES([Makefile
                 lib/Makefile
                 lib/libsysfs.pc
                 cmd/Makefile
                 test/Makefile])
AC_CONFIG_HEADERS([config.h])
AC_OUTPUT
