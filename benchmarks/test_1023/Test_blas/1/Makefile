CC = gcc
CFLAGS = -O3 -march=native -ffast-math -fopenmp
LDFLAGS ?=

# 根据你的安装路径修改
OPENBLAS_INC ?= /opt/OpenBLAS/include
OPENBLAS_LIB ?= /opt/OpenBLAS/lib
BLIS_INC     ?= /usr/local/include/blis
BLIS_LIB     ?= /usr/local/lib

OPENBLAS_LDLIBS ?= -lopenblas
BLIS_LDLIBS ?= -lblis
ARMPL_LDLIBS ?= -larmpl_lp64_mp -lm

.PHONY: all openblas blis armpl clean
all: openblas blis

openblas: dgemm_bench.c ../../../benchmark_args.h
	$(CC) $(CFLAGS) -I$(OPENBLAS_INC) $< -L$(OPENBLAS_LIB) -Wl,-rpath,$(OPENBLAS_LIB) $(OPENBLAS_LDLIBS) -o dgemm_bench_openblas $(LDFLAGS)

blis: dgemm_bench.c ../../../benchmark_args.h
	$(CC) $(CFLAGS) -I$(OPENBLAS_INC) $< -L$(BLIS_LIB) -Wl,-rpath,$(BLIS_LIB) $(BLIS_LDLIBS) -o dgemm_bench_blis $(LDFLAGS)

clean:
	rm -f dgemm_bench_openblas dgemm_bench_blis dgemm_bench_armpl


