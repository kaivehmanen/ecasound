# ---
# manual configuration
srcdir=..
# ---

ECAFLAGS= -I${srcdir}/libecasound -I${srcdir}/libecasoundc -D_REENTRANT -g -Wall -Wstrict-prototypes -DVERBOSE
ECALINK=-L${srcdir}/libecasound/.libs -lecasound_debug \
	-L${srcdir}/libecasoundc/.libs -lecasoundc_debug \
	-L${srcdir}/kvutils/.libs -lkvutils

CXXFLAGS += $(ECAFLAGS)
CFLAGS += $(ECAFLAGS)
LDFLAGS += $(ECALINK)
CXX=c++

COMMON_SRC = 		ecatestsuite.h
GENERATED_FILES = 	ecatestlist.txt
TARGETS=		con_test1 \
			con_test2 \
			eci_test1 \
			eci_test2 \
			eci_test3


all: $(TARGETS) create-test-list

create-test-list:
	echo $(TARGETS) > ecatestlist.txt

clean:
	rm -f $(TARGETS) $(GENERATED_FILES)
