CC = gcc
C++ = g++
CFLAGS = -O3 -march=native -ffast-math -fopenmp
LDFLAGS =

# 根据你的安装路径修改
OPENBLAS_INC ?= /opt/OpenBLAS/include
OPENBLAS_LIB ?= /opt/OpenBLAS/lib
BLIS_INC     ?= /usr/local/include/blis
BLIS_LIB     ?= /usr/local/lib
SHALOM_PREFIX ?= /opt/LibShalom
SHALOM_INC    ?= $(SHALOM_PREFIX)/SMM/include
SHALOM_LIB    ?= $(SHALOM_PREFIX)/SMM/lib
ARMPL_INC ?= /opt/arm/armpl_25.07_gcc/include
ARMPL_LIB ?= /opt/arm/armpl_25.07_gcc/lib

all: openblas blis

openblas: dgemm_bench.c
	$(CC) $(CFLAGS) -I$(OPENBLAS_INC) $< -L$(OPENBLAS_LIB) -lopenblas -o dgemm_bench_openblas $(LDFLAGS)

blis: dgemm_bench.c
	$(CC) $(CFLAGS) -I$(BLIS_INC) $< -L$(BLIS_LIB) -lblis -o dgemm_bench_blis $(LDFLAGS)

# shalom: dgemm_bench_shalom.c
# shalom: 1.cpp
# 	$(C++) $(CFLAGS) -I$(SHALOM_INC) $< -L$(SHALOM_LIB) -lsmm -fopenmp -o dgemm_bench_shalom_
# shalom: 2.cpp
# 	$(C++) $(CFLAGS) -I$(SHALOM_INC) $< -L$(SHALOM_LIB) -lsmm -fopenmp -o sgemm_bench_shalom

armpl: dgemm_bench.c
	$(CC) $(CFLAGS) -I$(ARMPL_INC) $< -L$(ARMPL_LIB) -larmpl_lp64 -lm -lomp -o dgemm_bench_armpl $(LDFLAGS)

clean:
	rm -f dgemm_bench_openblas dgemm_bench_blis


