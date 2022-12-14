AC_PREREQ(2.60)
AC_INIT([spider],
        [1.0],
        [yaka@assistants.epita.fr],
        [spider],
        [https://assistants.epita.fr/])

AC_CONFIG_SRCDIR([src/main.cc])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([
                  foreign
                  -Wall
                  -Werror
                  silent-rules
                  subdir-objects
])

PKG_CHECK_MODULES([CRITERION], [criterion], [HAVE_CRITERION=1], [HAVE_CRITERION=0])
AM_CONDITIONAL([USING_CRITERION], [test "$HAVE_CRITERION" -eq 1])
AM_COND_IF([USING_CRITERION], [
                           AC_SUBST([CRITERION_CFLAGS])
                           AC_SUBST([CRITERION_LIBS])
])

AC_ARG_ENABLE([debug],
[  --enable-debug    Turn on debugging],
[case "${enableval}" in
  yes) debug=true ;;
  no)  debug=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],[debug=false])
AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])


AM_PROG_AR
AX_CXX_COMPILE_STDCXX_17([noext], [mandatory])

AX_CHECK_COMPILE_FLAG([-pedantic],
                      [PICKY_CXXFLAGS="-pedantic"])

AX_COMPILER_FLAGS

LT_INIT([
         disable-shared
         pic-only
 ])

AX_BOOST_BASE

AM_SILENT_RULES([yes])

AC_CONFIG_HEADERS(config.h)
AC_CONFIG_FILES([Makefile])

AC_SUBST([PICKY_CXXFLAGS])
AC_OUTPUT
