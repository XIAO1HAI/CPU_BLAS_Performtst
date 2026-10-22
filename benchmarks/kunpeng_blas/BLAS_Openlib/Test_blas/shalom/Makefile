LibShalom_PREFIX = /opt/LibShalom
LibShalom_INC    = $(LibShalom_PREFIX)/SMM/include
LibShalom_LIB    = $(LibShalom_PREFIX)/SMM/lib/libsmm.a

OTHER_LIBS  =-fopenmp

CC          = g++
CFLAGS      = -O3 -I$(LibShalom_INC)
LINKER      = $(CC)

OBJS        = dgemm_bench_shalom.o

%.o: %.c
	 $(CC) $(CFLAGS) -c -fopenmp $< -o $@

all: $(OBJS)
	$(LINKER) $(OBJS) -I$(LibShalom_INC) $(LibShalom_LIB) $(OTHER_LIBS) -o dgemm_bench_shalom
# shalom: dgemm_bench_shalom.c
# 	$(CC) $(CFLAGS) -I$(LibShalom_INC) $< -L$(SHALOM_LIB) -lsmm -fopenmp -o dgemm_bench_shalom

# shalom: dgemm_bench_shalom.c
# 	$(CC) $(CFLAGS) $< -L$(LibShalom_LIB) -fopenmp -o dgemm_bench_shalom

clean:
	rm -f dgemm_bench_shalom