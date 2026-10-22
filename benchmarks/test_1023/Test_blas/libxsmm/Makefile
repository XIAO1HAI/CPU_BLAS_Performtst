CC = gcc
LIBXSMM_ROOT ?= /opt/libxsmm
LIBXSMM_INC ?= $(LIBXSMM_ROOT)/include
LIBXSMM_LIB ?= $(LIBXSMM_ROOT)/lib
LIBXSMM_LDLIBS ?= -lxsmm
CFLAGS ?= -O3 -march=native -ffast-math -fopenmp
LDFLAGS ?= -L$(LIBXSMM_LIB) -Wl,-rpath,$(LIBXSMM_LIB)
SRC ?= gemm_bench_libxsmm_nt.c
.PHONY: all clean
all: gemm_bench_libxsmm_nt
gemm_bench_libxsmm_nt: $(SRC) $(dir $(SRC))../../../benchmark_args.h
	$(CC) $(CFLAGS) -I$(LIBXSMM_INC) $< -o $@ $(LDFLAGS) $(LIBXSMM_LDLIBS) -lm -ldl -lpthread -fopenmp
clean:
	rm -f gemm_bench_libxsmm_nt
