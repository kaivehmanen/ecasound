ECAFLAGS=-I../libecasound -I../libecasoundc -D_REENTRANT -g -Wall -Wstrict-prototypes -DVERBOSE
ECALINK=-L../libecasound/.libs -lecasound_debug -L../libecasoundc/.libs -lecasoundc_debug

CXXFLAGS += $(ECAFLAGS)
CFLAGS += $(ECAFLAGS)
LDFLAGS += $(ECALINK)

COMMON_SRC = 		ecatestsuite.h
GENERATED_FILES = 	ecatestlist.txt
TARGETS=		eci_test1 \
			eci_test2 \
			eci_test3

all: $(TARGETS) create-test-list

create-test-list:
	echo $(TARGETS) > ecatestlist.txt

clean:
	rm -f $(TARGETS) $(GENERATED_FILES)
