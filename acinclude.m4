AC_DEFUN([MP_WITH_CURSES],
  [AC_ARG_WITH(ncurses, [  --with-ncurses          Force the use of ncurses over curses],,)
   mp_save_LIBS="$LIBS"
   CURSES_LIB=""
   if test "$with_ncurses" != yes
   then
     AC_CACHE_CHECK([for working curses], mp_cv_curses,
       [LIBS="$LIBS -lcurses"
        AC_TRY_LINK(
          [#include <curses.h>],
          [chtype a; int b=A_STANDOUT, c=KEY_LEFT; initscr(); ],
          mp_cv_curses=yes, mp_cv_curses=no)])
     if test "$mp_cv_curses" = yes
     then
       AC_DEFINE(HAVE_CURSES_H, 1, [Define to 1 if libcurses is requested])
       #AC_DEFINE(HAVE_CURSES_H)
       CURSES_LIB="-lcurses"
     fi
   fi
   if test ! "$CURSES_LIB"
   then
     AC_CACHE_CHECK([for working ncurses], mp_cv_ncurses,
       [LIBS="$mp_save_LIBS -lncurses"
        AC_TRY_LINK(
          [#include <ncurses.h>],
          [chtype a; int b=A_STANDOUT, c=KEY_LEFT; initscr(); ],
          mp_cv_ncurses=yes, mp_cv_ncurses=no)])
     if test "$mp_cv_ncurses" = yes
     then
       AC_DEFINE(HAVE_NCURSES_H, 1, [Define to 1 if libncurses is requested])
       CURSES_LIB="-lncurses"
     fi
   fi
   if test ! "$CURSES_LIB"
   then
     AC_CACHE_CHECK([for working pdcurses], mp_cv_pdcurses,
       [LIBS="$mp_save_LIBS -lpdcurses"
        AC_TRY_LINK(
          [#include <curses.h>],
          [chtype a; int b=A_STANDOUT, c=KEY_LEFT; initscr(); ],
          mp_cv_pdcurses=yes, mp_cv_pdcurses=no)])
     if test "$mp_cv_pdcurses" = yes
     then
       AC_DEFINE(HAVE_CURSES_H, 1, [Define to 1 if libcurses is requested])
       CURSES_LIB="-lpdcurses"
     fi
   fi
   if test ! "$CURSES_LIB"
   then
     AC_CACHE_CHECK([for working local pdcurses], mp_cv_lpdcurses,
       [LIBS="$mp_save_LIBS ../pdcurs33/pdcurses.a"
        AC_TRY_LINK(
          [#include "../pdcurs33/curses.h"],
          [chtype a; int b=A_STANDOUT, c=KEY_LEFT; initscr(); ],
          mp_cv_lpdcurses=yes, mp_cv_lpdcurses=no)])
     if test "$mp_cv_lpdcurses" = yes
     then
       AC_DEFINE(HAVE_CURSES_H, 1, [Define to 1 if libcurses is requested])
       CURSES_LIB="../pdcurs33/pdcurses.a"
       RF_ADDTO(CPPFLAGS,"-I../pdcurs33")
     fi
   fi
   if test ! "$CURSES_LIB" ; then
       LIBS="$mp_save_LIBS"
   fi
])dnl

dnl
dnl RF_ADDTO(variable, value)
dnl
dnl  Add value to variable
dnl
AC_DEFUN(RF_ADDTO,[
  if test "x$$1" = "x"; then
    test "x$silent" != "xyes" && echo "  setting $1 to \"$2\""
    $1="$2"
  else
    apr_addto_bugger="$2"
    for i in $apr_addto_bugger; do
      apr_addto_duplicate="0"
      for j in $$1; do
        if test "x$i" = "x$j"; then
          apr_addto_duplicate="1"
          break
        fi
      done
      if test $apr_addto_duplicate = "0"; then
        test "x$silent" != "xyes" && echo "  adding \"$i\" to $1"
        $1="$$1 $i"
      fi
    done
  fi
])dnl
