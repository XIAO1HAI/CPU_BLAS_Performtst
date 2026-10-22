CC = gcc
CFLAGS  = -O3 -march=native -ffast-math -fopenmp -I/opt/libxsmm/include
LDFLAGS = -L/opt/libxsmm/lib -lxsmm -fopenmp
# 如果你的安装前缀不同，改上面两个路径即可

all: gemm_bench_libxsmm_nt

gemm_bench_libxsmm_nt: gemm_bench_libxsmm_nt.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f gemm_bench_libxsmm_nt

