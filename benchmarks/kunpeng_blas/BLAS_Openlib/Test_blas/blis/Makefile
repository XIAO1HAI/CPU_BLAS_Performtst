# # CC = gcc
# # CFLAGS = -O3 -DNDEBUG -march=native -fopenmp -I$(BLIS_HOME)/include/blis
# # LDFLAGS = -L$(BLIS_HOME)/lib -lblis -lm -lpthread
# CC = gcc
# C++ = g++
# CFLAGS = -O3 -march=native -ffast-math -fopenmp
# BLIS_INC     ?= /usr/local/include/blis
# BLIS_LIB     ?= /usr/local/lib

# all: dgemm_nt_bench_blis

# # dgemm_nt_bench_blis: dgemm_nt_bench_blis.c
# # 	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# blis: dgemm_nt_bench_blis.c
# 	$(CC) $(CFLAGS) -I$(BLIS_INC) $< -L$(BLIS_LIB) -lblis -o dgemm_bench_blis $(LDFLAGS)

# clean:
# 	rm -f dgemm_nt_bench_blis



CC = gcc
CFLAGS  = -O3 -DNDEBUG -march=native -fopenmp \
          -I$(BLIS_HOME)/include/blis -I$(BLIS_HOME)/include
LDFLAGS = -L$(BLIS_HOME)/lib -Wl,-rpath,$(BLIS_HOME)/lib -lblis -lm -lpthread

all: dgemm_nt_bench_blis

dgemm_nt_bench_blis: dgemm_nt_bench_blis.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f dgemm_nt_bench_blis
