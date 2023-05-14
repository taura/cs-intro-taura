CC := gcc
CXX := g++
CFLAGS :=
CFLAGS += -O3
# CFLAGS += -O0
# CFLAGS += -g
CFLAGS += -fopenmp
CFLAGS += -march=native
CFLAGS += -mavx512f
CXXFLAGS := -O3

c := $(wildcard *.c)
cc := $(wildcard *.cc)
c_asm := $(patsubst %.c,%.s,$(c))
cc_asm := $(patsubst %.cc,%.s,$(cc))
c_exe := $(patsubst %.c,%,$(c))
cc_exe := $(patsubst %.cc,%,$(cc))
targets :=
targets += $(c_exe)
targets += $(cc_exe)
targets += $(c_asm)
targets += $(cc_asm)

all : $(targets)

$(c_exe) : % : %.c
	$(CC) -o $@ $(CFLAGS) $<

$(cc_exe) : % : %.cc
	$(CXX) -o $@ $(CXXFLAGS) $<

$(c_asm) : %.s : %.c
	$(CC) -o $@ -S $(CFLAGS) $<

$(cc_asm) : %.s : %.cc
	$(CXX) -o $@ -S $(CXXFLAGS) $<

clean :
	rm -f $(c_exe) $(cc_exe) $(c_asm) $(cc_asm)
