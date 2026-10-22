# Makefile (threaded-first for ARMPL)
ARMPL_DIR ?= /opt/arm/armpl_25.07_gcc
# GCC 版通常是 gomp；若你装的是 LLVM/Clang 版，请用 OMP_LIB=omp
OMP_LIB ?= gomp

CC ?= gcc
CFLAGS  = -O3 -march=native -ffast-math -fopenmp -I$(ARMPL_DIR)/include
LDFLAGS = -L$(ARMPL_DIR)/lib -lm -l$(OMP_LIB)

# 优先并行库；若没这个文件，就回退到 lp64
ARMPL_THREADED  = $(ARMPL_DIR)/lib/libarmpl_lp64_mp.so
ifeq ($(wildcard $(ARMPL_THREADED)),)
  ARMPL_LIB = -larmpl_lp64
else
  ARMPL_LIB = -larmpl_lp64_mp
endif

SRC    = armpl_bench_dgemm.c
TARGET = armpl_bench_dgemm

all: $(TARGET)
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(ARMPL_LIB) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o

print:
	@echo "ARMPL_DIR=$(ARMPL_DIR)"
	@echo "OMP_LIB=$(OMP_LIB)"
	@echo "ARMPL_LIB=$(ARMPL_LIB)"

