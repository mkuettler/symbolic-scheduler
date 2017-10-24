
CXX ?= clang++
CXXFLAGS = -std=c++14 -g -MMD -O2
#if $(eq CXX, clang++)
#	CXXFLAGS += -Wall -Wmost -Werror
#else
	CXXFLAGS += -Wall -Wextra -Werror -Wno-sign-compare #-fsanitize=address -fsanitize=undefined
#	CXXLIBS += -lasan -lubsan
#fi

LDFLAGS = -lpthread

SRC_SIM = simulator.cc main-simulator.cc task.cc scheduler.cc job.cc time.cc
OBJ_SIM = $(SRC_SIM:.cc=.o)

SRC_SYM = symbolic_scheduler.cc main.cc task.cc scheduler.cc job.cc time.cc
OBJ_SYM = $(SRC_SYM:.cc=.o)

DEPS = $(SRC_SIM:.cc=.d) $(SRC_SYM:.cc=.d)

all: simulator symbolic_scheduler

%.o: %.cc Makefile
	$(CXX) $(CXXFLAGS) -c $< $(CXXLIBS) -o $@

simulator: $(OBJ_SIM)
	$(CXX) $(CXXFLAGS) $(CXXLIBS) $^ -o $@

symbolic_scheduler: $(OBJ_SYM)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(CXXLIBS) $^ -o $@

clean:
	rm -f $(OBJ_SIM) $(OBJ_SYM) simulator symbolic_scheduler $(DEPS)

-include $(DEPS)
