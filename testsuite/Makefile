# ---
# manual configuration
srcdir=..
#ecasound_library=ecasound8
ecasound_library=ecasound8_debug
#ecasoundc_library=ecasoundc2
ecasoundc_library=ecasoundc2_debug
#kvutils_library=kvutils3
kvutils_library=kvutils3_debug

# ---

ECAFLAGS= -I${srcdir}/libecasound -I${srcdir}/kvutils -I${srcdir}/libecasoundc -D_REENTRANT -g -Wall -Wstrict-prototypes -DVERBOSE
ECALINK=-L${srcdir}/libecasoundc/.libs -l${ecasoundc_library} \
	-L${srcdir}/libecasound/.libs -l${ecasound_library} \
	-L${srcdir}/kvutils/.libs -l${kvutils_library} \
	-Wl,--rpath -Wl,${srcdir}/libecasoundc/.libs \
	-Wl,--rpath -Wl,${srcdir}/libecasound/.libs \
	-Wl,--rpath -Wl,${srcdir}/kvutils/.libs

CXXFLAGS += $(ECAFLAGS)
CFLAGS += $(ECAFLAGS)
LDFLAGS += $(ECALINK)
CXX=c++
CC=c++

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
