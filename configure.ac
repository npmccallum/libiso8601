AC_INIT([libiso8601], [0.1])
AC_CONFIG_MACRO_DIR([m4])
AC_PREFIX_DEFAULT([/usr])
AC_PREREQ([2.59])

AM_INIT_AUTOMAKE([1.10 no-define])
AM_SILENT_RULES([yes])

LT_INIT()

AC_PROG_CC_C99
AC_CHECK_SIZEOF(time_t)


CFLAGS="$CFLAGS \
-Wall \
-Wchar-subscripts \
-Wformat-security \
-Wmissing-declarations \
-Wmissing-prototypes \
-Wnested-externs \
-Wpointer-arith \
-Wshadow \
-Wsign-compare \
-Wstrict-prototypes \
-Wtype-limits \
"

AC_CONFIG_FILES([Makefile tests/Makefile])
AC_OUTPUT
AC_MSG_RESULT([
        $PACKAGE $VERSION
        =====

        prefix:                 ${prefix}
        sysconfdir:             ${sysconfdir}
        libdir:                 ${libdir}
        includedir:             ${includedir}

        compiler:               ${CC}
        cflags:                 ${CFLAGS}
        ldflags:                ${LDFLAGS}

        logging:                ${enable_logging}
        debug:                  ${enable_debug}
])
