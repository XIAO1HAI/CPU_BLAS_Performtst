CXX = g++
LibShalom_PREFIX ?= /opt/LibShalom
LibShalom_INC ?= $(LibShalom_PREFIX)/SMM/include
LibShalom_LIB ?= $(LibShalom_PREFIX)/SMM/lib/libsmm.a
CXXFLAGS ?= -O3 -march=native -fopenmp
.PHONY: all clean
all: dgemm_bench_shalom
dgemm_bench_shalom: dgemm_bench_shalom.c ../../../../benchmark_args.h
	$(CXX) $(CXXFLAGS) -I$(LibShalom_INC) $< $(LibShalom_LIB) -fopenmp -o $@
clean:
	rm -f dgemm_bench_shalom *.o
