# ---
# manual configuration
srcdir=..
ecasound_library=ecasound
#ecasound_library=ecasound_debug
ecasoundc_library=ecasoundc
#ecasoundc_library=ecasoundc_debug
# ---

ECAFLAGS= -I${srcdir}/libecasound -I${srcdir}/libecasoundc -D_REENTRANT -g -Wall -Wstrict-prototypes -DVERBOSE
ECALINK=-L${srcdir}/libecasound/.libs -l${ecasound_library} \
	-L${srcdir}/libecasoundc/.libs -l${ecasoundc_library} \
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
