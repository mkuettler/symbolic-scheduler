
CXX ?= clang++
CXXFLAGS = -std=c++14 -g -MMD -O1
#if $(eq CXX, clang++)
#	CXXFLAGS += -Wall -Wmost -Werror
#else
	CXXFLAGS += -Wall -Wextra -Werror -Wno-sign-compare #-fsanitize=address -fsanitize=undefined
#	CXXLIBS += -lasan -lubsan
#fi

LDFLAGS = -lpthread

SRC_SIM = simulator.cc main.cc task.cc scheduler.cc job.cc time.cc
OBJ_SIM = $(SRC_SIM:.cc=.o)

SRC_RSM = real-simulator.cc main-real-sim.cc task.cc scheduler.cc job.cc time.cc
OBJ_RSM = $(SRC_RSM:.cc=.o)

DEPS = $(SRC_SIM:.cc=.d) $(SRC_CLD:.cc=.d) $(SRC_RSM:.cc=.d)

all: simulator realsim

%.o: %.cc Makefile
	$(CXX) $(CXXFLAGS) -c $< $(CXXLIBS) -o $@

simulator: $(OBJ_SIM)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) $(CXXLIBS) $^ -o $@

realsim: $(OBJ_RSM)
	$(CXX) $(CXXFLAGS) $(CXXLIBS) $^ -o $@

clean:
	rm -f $(OBJ_SIM) $(OBJ_RSM) simulator realsim $(DEPS)

-include $(DEPS)
