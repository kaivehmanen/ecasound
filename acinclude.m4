dnl ---
dnl acinclude.m4 for ecasound
dnl last modified: 29.04.2002
dnl ---

## ------------------------------------------------------------------------
## Check for LFS
## 
## version: 2
##
## modifies: CXXFLAGS, CFLAGS
## ------------------------------------------------------------------------
##

AC_DEFUN(AC_CHECK_LARGEFILE,
[
AC_MSG_CHECKING(for largefile support (>2GB files))
AC_ARG_WITH(largefile,
  [  --with-largefile        Support large (>2GB) files],
  [ if test "x$withval" = "xyes" ; then
      enable_largefile="yes"
    fi 
  ])
       
if test "x$enable_largefile" = "xyes"; then
  # AC_DEFINE(_FILE_OFFSET_BITS, 64)
  # AC_DEFINE(_LARGEFILE_SOURCE)
  CXXFLAGS="$CXXFLAGS -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE"
  CFLAGS="$CFLAGS -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE"
  AC_MSG_RESULT(yes.)
else
  AC_MSG_RESULT(no.)
fi

])

## ------------------------------------------------------------------------
## Check whether namespaces are supported.
##
## version: 3
##
## defines: ECA_USE_CXX_STD_NAMESPACE
## ------------------------------------------------------------------------
##
AC_DEFUN(AC_CHECK_CXX_NAMESPACE_SUPPORT,
[
AC_MSG_CHECKING(if C++ compiler supports namespaces)
AC_LANG_CPLUSPLUS
old_cxx_flags=$CXXFLAGS
CXXFLAGS="-fno-exceptions $CXXFLAGS" # hack around gcc3.x feature
AC_TRY_RUN(
[
#include <string>
#include <vector>

using std::string;

int main(void)
{	
	string s ("foo");
 	std::vector<string> v;
	return(0);
}
],
[ 	
	AC_MSG_RESULT(yes.)
	AC_DEFINE(ECA_USE_CXX_STD_NAMESPACE)
],
[
	AC_MSG_RESULT(no.)
	AC_MSG_WARN([C++ compiler has problems with namespaces. Build process can fail because of this.])
]
,
[
	AC_MSG_RESULT(no.)
]
)
CXXFLAGS=$old_cxx_flags
])

## ------------------------------------------------------------------------
## Find a file (or one of more files in a list of dirs)
##
## version: 1
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
