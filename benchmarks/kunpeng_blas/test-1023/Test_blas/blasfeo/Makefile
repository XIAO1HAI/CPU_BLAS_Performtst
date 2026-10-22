# Makefile for BLASFEO dgemm NT benchmark (OpenMP outer parallel)

BLASFEO_PREFIX ?= /usr/local
CC      ?= gcc
CFLAGS   = -O3 -march=native -ffast-math -fopenmp -I$(BLASFEO_PREFIX)/include
LDFLAGS  = -L$(BLASFEO_PREFIX)/lib
LDLIBS   = -lblasfeo -lm -fopenmp

.PHONY: all clean
all: bench_blasfeo_dgemm_nt

bench_blasfeo_dgemm_nt: bench_blasfeo_dgemm_nt.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f bench_blasfeo_dgemm_nt
