# ---
# manual configuration
srcdir=../..
#ecasound_library=ecasound_debug
ecasound_library=ecasound_debug
#kvutils_library=kvutils_debug
kvutils_library=kvutils_debug

# ---

ECAFLAGS= -I${srcdir}/libecasound -I${srcdir}/kvutils -I${srcdir}/libecasoundc -D_REENTRANT -g -Wall -Wstrict-prototypes -DVERBOSE
ECALDFLAGS= \
	-L${srcdir}/libecasound/.libs \
	-L${srcdir}/kvutils/.libs

ECALIBS= \
	${srcdir}/libecasound/.libs/lib${ecasound_library}.a \
	${srcdir}/kvutils/.libs/lib${kvutils_library}.a

CXXFLAGS += $(ECAFLAGS)
CFLAGS += $(ECACFLAGS)
LDFLAGS += $(ECALDFLAGS)

CXX=c++
CC=c++

# FIXME: move C eci tests to libecasoundc?

COMMON_SRC = 		ecatestsuite.h
GENERATED_FILES = 	ecatestlist.txt
TARGETS=		con_test1 \
			con_test2

#			eca_test1

all: $(TARGETS) create-test-list

con_test1: con_test1.o ${ECALIBS}
con_test2: con_test2.o ${ECALIBS}
eca_test1: ${ECALIBS} eca_test1.o

create-test-list:
	echo $(TARGETS) > ecatestlist.txt

clean:
	rm -fv $(TARGETS) $(GENERATED_FILES) *.o
