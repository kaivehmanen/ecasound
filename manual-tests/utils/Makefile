# ----------------------------------------------------------------------
# File: ecasound/manual-tests/utils/Makefile
#
# License: GPL (see ecasound/{AUTHORS,COPYING})
# ----------------------------------------------------------------------

CPPFLAGS = `sh ../../libecasound/libecasound-config --cflags`
LIBS_DEBUG = -L../../libecasound/.libs -L../../kvutils `sh ../../libecasound/libecasound-config --libs_debug`
LIBS = -L../../libecasound/.libs -L../../kvutils `sh ../../libecasound/libecasound-config --libs`

all: ecacompare

ecacompare: ecacompare.cpp
	( c++ -o ecacompare ecacompare.cpp $(CPPFLAGS) $(LIBS) || \
	 echo "Compiling with debug libs instead: " && \
	 c++ -o ecacompare ecacompare.cpp $(CPPFLAGS) $(LIBS_DEBUG) )

clean: 
	rm -vf ecacompare

.PHONY: all clean


