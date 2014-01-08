ACLOCAL_AMFLAGS = -I m4
SUBDIRS = . tests

noinst_LTLIBRARIES = libinternal.la
libinternal_la_SOURCES = src/internal.c src/internal.h

lib_LTLIBRARIES = libiso8601.la
libiso8601_la_LDFLAGS = -export-symbols libiso8601.sym
libiso8601_la_LIBADD = libinternal.la
libiso8601_la_SOURCES =	src/parse.c src/unparse.c src/add.c src/misc.c

