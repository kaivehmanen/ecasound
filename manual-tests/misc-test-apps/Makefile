ECAFLAGS = $(shell libecasound-config --cflags) -I.
ECALDFLAGS = $(shell libecasound-config --ldflags)
#ECALIBS = $(shell libecasound-config --libs)
ECALIBS = $(shell libecasound-config --libs_debug)

CXX=c++
CPPFLAGS += -DVERBOSE
CPPFLAGS += -DLIBECASOUND_VERSION=$(shell grep libecasound_version `which libecasound-config` |cut -f2 -d=)
CXXFLAGS += -Werror -O2 -g

# FIXME: move C eci tests to libecasoundc?

COMMON_SRC = 		ecatestsuite.h
GENERATED_FILES = 	ecatestlist.txt

LIB_TESTS=		eca_loop_bench$(EXEEXT)

EXEC_TESTS=		con_test1 \
			con_test2
ALL_TESTS=		$(LIB_TESTS) $(EXEC_TESTS)

# not built:
#			eca_test1

all: $(ALL_TESTS) ecatestlist.txt

con_test1: con_test1.o
con_test2: con_test2.o
eca_loop_bench$(EXEEXT): eca_loop_bench$(EXEEXT).o

%$(EXEEXT).o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(ECAFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(ECAFLAGS) -c -o $@ $<

$(ALL_TESTS):
	$(CXX) $(LDFLAGS) $(ECALDFLAGS) -o $@ $@.o $(ECALIBS)

ecatestlist.txt: $(ALL_TESTS)
	echo $(ALL_TESTS) > ecatestlist.txt

clean:
	rm -fv $(ALL_TESTS) $(GENERATED_FILES) *.o
