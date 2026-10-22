# Makefile for BLIS DGEMM-NT benchmark
BLIS_DIR ?= /usr/local
BLIS_INC ?= $(BLIS_DIR)/include/blis
BLIS_LIB ?= $(BLIS_DIR)/lib

# 线程实现：openmp 或 pthreads（按你的 BLIS 构建来）
BLIS_THREAD_IMPL ?= openmp

CC      ?= gcc
CFLAGS   = -O3 -march=native -ffast-math -I$(BLIS_INC)
LDFLAGS  = -L$(BLIS_LIB) -lblis -lm

ifeq ($(BLIS_THREAD_IMPL),openmp)
  CFLAGS  += -fopenmp
else ifeq ($(BLIS_THREAD_IMPL),pthreads)
  LDFLAGS += -lpthread
endif

SRC    = blis_bench_dgemm.c
TARGET = blis_bench_dgemm

all: $(TARGET)
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o

print:
	@echo "BLIS_INC=$(BLIS_INC)"
	@echo "BLIS_LIB=$(BLIS_LIB)"
	@echo "BLIS_THREAD_IMPL=$(BLIS_THREAD_IMPL)"
	
