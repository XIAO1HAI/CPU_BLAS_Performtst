CC = gcc
BLIS_HOME ?= /usr/local
BLIS_INC ?= $(BLIS_HOME)/include/blis
BLIS_LIB ?= $(BLIS_HOME)/lib
BLIS_LDLIBS ?= -lblis
CFLAGS ?= -O3 -march=native -fopenmp
LDFLAGS ?= -L$(BLIS_LIB) -Wl,-rpath,$(BLIS_LIB)
LDLIBS = $(BLIS_LDLIBS) -lm -lpthread -fopenmp
.PHONY: all blis clean
all: dgemm_nt_bench_blis dgemm_bench_blis
blis: dgemm_bench_blis
dgemm_nt_bench_blis: dgemm_nt_bench_blis.c ../../../../benchmark_args.h
	$(CC) $(CFLAGS) -I$(BLIS_INC) $< -o $@ $(LDFLAGS) $(LDLIBS)
dgemm_bench_blis: ../dgemm_bench.c ../../../../benchmark_args.h
	$(CC) $(CFLAGS) -I$(BLIS_INC) $< -o $@ $(LDFLAGS) $(LDLIBS)
clean:
	rm -f dgemm_nt_bench_blis dgemm_bench_blis
