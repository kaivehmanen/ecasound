## ------------------------------------------------------------------------
## Find the meta object compiler in the PATH, in $QTDIR/bin, and some
## more usual places
## ------------------------------------------------------------------------
##
AC_DEFUN(AC_PATH_QT_MOC,
[
AC_PATH_PROG(MOC, moc, /usr/bin/moc,
 $ac_qt_bindir:$QTDIR/bin:$PATH:/usr/bin:/usr/X11R6/bin:/usr/lib/qt/bin:/usr/local/qt/bin)
])


## ------------------------------------------------------------------------
## Try to find the QT headers and libraries.
## ------------------------------------------------------------------------
##
AC_DEFUN(AC_PATH_QT,
[

qt_incdirs="$ac_qt_includes /usr/lib/qt/include /usr/local/qt/include /usr/include/qt /usr/include /usr/X11R6/include/X11/qt $x_includes $QTINC"

test -n "$QTDIR" && qt_incdirs="$QTDIR/include $QTDIR $qt_incdirs"
AC_FIND_FILE(qmovie.h, $qt_incdirs, qt_incdir)

ac_qt_includes="$qt_incdir"
qt_includes="$ac_qt_includes"

AC_SUBST(qt_includes)

QT_INCLUDES="-I$qt_includes"
AC_SUBST(QT_INCLUDES)
])

## ------------------------------------------------------------------------
## Find a file (or one of more files in a list of dirs)
## ------------------------------------------------------------------------
##
AC_DEFUN(AC_FIND_FILE,
[
$3=NO
for i in $2;
do
  for j in $1;
  do
    if test -r "$i/$j"; then
      $3=$i
      break 2
    fi
  done
done
])
