/** 
 * acconfig.h for ecasound
 * last modified: 25.04.2002
 */

/* Package name */
#undef PACKAGE

/* Package version */
#undef VERSION

/* libecasound interface version */
#undef LIBECASOUND_VERSION

/* libecasoundc interface version */
#undef LIBECASOUNDC_VERSION

/* libkvutils interface version */
#undef LIBKVUTILS_VERSION

/* Ecasound configure script prefix */
#undef ECA_PREFIX

/* use libncurses for termcap info */
#undef ECA_USE_NCURSES

/* ncurses headers are installed in ncurses subdir <ncurses/curses.h> */
#undef ECA_HAVE_NCURSES_CURSES_H

/* use libtermcap for termcap info */
#undef ECA_USE_TERMCAP

/* whether to compile OSS input/output */
#undef ECA_COMPILE_OSS

/* whether to use trigger OSS trigger functions */
#undef ECA_DISABLE_OSS_TRIGGER

/* well, are you? :) */
#undef ECA_FEELING_EXPERIMENTAL

/* whether to disable all use of shared libs */
#undef ECA_STATIC_ONLY

/* use locking primitives found in asm/atomic.h */
#undef ECA_USE_ASM_ATOMIC

/* use C++ std namespace */
#undef ECA_USE_CXX_STD_NAMESPACE
