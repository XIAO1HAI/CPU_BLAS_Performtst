CC = gcc
CXX = g++
CFLAGS = -O3 -march=native -ffast-math -fopenmp
LDFLAGS ?=

OPENBLAS_INC ?= /opt/OpenBLAS/include
OPENBLAS_LIB ?= /opt/OpenBLAS/lib
BLIS_INC     ?= /usr/local/include/blis
BLIS_LIB     ?= /usr/local/lib
SHALOM_PREFIX ?= /opt/LibShalom
SHALOM_INC    ?= $(SHALOM_PREFIX)/SMM/include
SHALOM_LIB    ?= $(SHALOM_PREFIX)/SMM/lib
ARMPL_INC ?= /opt/arm/armpl_25.07_gcc/include
ARMPL_LIB ?= /opt/arm/armpl_25.07_gcc/lib

OPENBLAS_LDLIBS ?= -lopenblas
BLIS_LDLIBS ?= -lblis
ARMPL_LDLIBS ?= -larmpl_lp64_mp -lm

.PHONY: all openblas blis armpl clean
all: armpl

openblas: dgemm_bench.c ../../../../benchmark_args.h
	$(CC) $(CFLAGS) -I$(OPENBLAS_INC) $< -L$(OPENBLAS_LIB) -Wl,-rpath,$(OPENBLAS_LIB) $(OPENBLAS_LDLIBS) -o dgemm_bench_openblas $(LDFLAGS)

blis: dgemm_bench.c ../../../../benchmark_args.h
	$(CC) $(CFLAGS) -I$(BLIS_INC) $< -L$(BLIS_LIB) -Wl,-rpath,$(BLIS_LIB) $(BLIS_LDLIBS) -o dgemm_bench_blis $(LDFLAGS)


armpl: dgemm_bench.c ../../../../benchmark_args.h
	$(CC) $(CFLAGS) -I$(ARMPL_INC) $< -L$(ARMPL_LIB) -Wl,-rpath,$(ARMPL_LIB) $(ARMPL_LDLIBS) -o dgemm_bench_armpl $(LDFLAGS)

clean:
	rm -f dgemm_bench_openblas dgemm_bench_blis dgemm_bench_armpl

