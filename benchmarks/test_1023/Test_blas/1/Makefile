CC = gcc
CFLAGS = -O3 -march=native -ffast-math -fopenmp
LDFLAGS =

# 根据你的安装路径修改
OPENBLAS_INC ?= /opt/OpenBLAS/include
OPENBLAS_LIB ?= /opt/OpenBLAS/lib
BLIS_INC     ?= /usr/local/include/blis
BLIS_LIB     ?= /usr/local/lib

all: openblas blis

openblas: dgemm_bench.c
	$(CC) $(CFLAGS) -I$(OPENBLAS_INC) $< -L$(OPENBLAS_LIB) -lopenblas -o dgemm_bench_openblas $(LDFLAGS)

blis: dgemm_bench.c
	$(CC) $(CFLAGS) -I$(OPENBLAS_INC) $< -L$(BLIS_LIB) -lblis -o dgemm_bench_blis $(LDFLAGS)

clean:
	rm -f dgemm_bench_openblas dgemm_bench_blis


