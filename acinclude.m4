dnl ---
dnl acinclude.m4 for ecasound
dnl last modified: 20030807-4
dnl ---

## ------------------------------------------------------------------------
## Check for JACK support
##
## defines: ECA_AM_COMPILE_JACK, ECA_S_JACK_LIBS, ECA_S_JACK_INCLUDES,
##          ECA_COMPILE_JACK, ECA_JACK_TRANSPORT_API
## ------------------------------------------------------------------------

AC_DEFUN(AC_CHECK_JACK,
[
AC_CHECK_HEADER(jack/jack.h,jack_support=yes,jack_support=no)
AC_ARG_WITH(jack,
	    [  --with-jack=DIR	Compile against JACK installed in DIR],
	    [
	        ECA_S_JACK_LIBS="-L${withval}/lib"
		ECA_S_JACK_INCLUDES="-I${withval}/include"
		jack_support=yes
	    ])
AC_ARG_ENABLE(jack,
	      [  --disable-jack		  Disable JACK support (default = no)],
	      jack_support=no)
AM_CONDITIONAL(ECA_AM_COMPILE_JACK, test x$jack_support = xyes)

if test x$jack_support = xyes; then
    ECA_S_JACK_LIBS="${ECA_S_JACK_LIBS} -ljack -lrt"
    AC_DEFINE(ECA_COMPILE_JACK)
fi                                     

AC_LANG_C
old_cppflags=$CPPFLAGS
old_ldflags=$LDFLAGS
old_INCLUDES=$INCLUDES
CPPFLAGS="$CPPFLAGS $ECA_S_JACK_INCLUDES"
LDFLAGS="$LDFLAGS $ECA_S_JACK_LIBS"
INCLUDES="--host=a.out-i386-linux"

AC_TRY_LINK(
[ #include <jack/transport.h> ],
[
	jack_position_t t;
	int *a = (void*)&jack_transport_query;
	int *b = (void*)&jack_transport_start;
	int *c = (void*)&jack_transport_stop;
	t.frame = 0;
	t.valid = 0;
	return 0;
],
[ ECA_JACK_TRANSPORT_API="3" ],
[ ECA_JACK_TRANSPORT_API="2" ]
)

AC_TRY_LINK(
[ #include <jack/transport.h> ],
[
	jack_transport_info_t t;
	t.state = 0;
	return 0;
],
[ ECA_JACK_TRANSPORT_API="1" ],
[ true ]
)

CPPFLAGS="$old_cppflags"
LDFLAGS="$old_ldflags"
INCLUDES="$old_INCLUDES"

echo "Using JACK transport API version:" ${ECA_JACK_TRANSPORT_API}
AC_DEFINE_UNQUOTED(ECA_JACK_TRANSPORT_API, ${ECA_JACK_TRANSPORT_API})

AC_SUBST(ECA_S_JACK_LIBS)
AC_SUBST(ECA_S_JACK_INCLUDES)
])

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
