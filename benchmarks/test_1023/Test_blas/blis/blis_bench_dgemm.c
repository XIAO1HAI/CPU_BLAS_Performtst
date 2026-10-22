// armpl_bench_dgemm.c  (force-parallel version)
#include "../../../benchmark_args.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <cblas.h>
#include <omp.h>
#include <unistd.h>

static inline double now_sec(void){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}
static void init_matrix(double *a, size_t n, unsigned seed){
    srand(seed);
    for (size_t i = 0; i < n; ++i) a[i] = ((rand()%200)/100.0) - 1.0;
}
static int env_to_int(const char* k, int defv){
    const char* v = getenv(k); return v ? bench_int(v, 1) : defv;
}
static void usage(const char* p){
    fprintf(stderr,"Usage: %s M N K [warmup=1] [repeat=7]\n", p);
}

int main(int argc, char** argv){
    if (argc < 4) { usage(argv[0]); return 1; }
    const int M = bench_int(argv[1], 1), N = bench_int(argv[2], 1), K = bench_int(argv[3], 1);
    const int warmup = (argc>=5)?bench_int(argv[4], 0):1, repeat = (argc>=6)?bench_int(argv[5], 1):7;

    // ---- 强制 OpenMP 并行设置（影响库内部）----
    int th = env_to_int("OMP_NUM_THREADS", -1);
    if (th > 0) omp_set_num_threads(th);
    omp_set_dynamic(0);                 // 禁止动态降线程
    omp_set_max_active_levels(1);       // 防止嵌套干扰

    // 打印环境
    const char* env_threads = getenv("OMP_NUM_THREADS"); if (!env_threads) env_threads="env_default";
    fprintf(stderr,"# omp_get_max_threads=%d, OMP_NUM_THREADS=%s\n", omp_get_max_threads(), env_threads);

    // ---- 1s 并行探针（在 htop 能看到 N 核拉高）----
    double t0 = omp_get_wtime();
    #pragma omp parallel
    {
        volatile double s = 0;
        while (omp_get_wtime() - t0 < 1.0) s += 1.0;
    }

    // ---- 分配/初始化 ----
    size_t szA=bench_elements(M,K), szB=bench_elements(K,N), szC=bench_elements(M,N);
    double *A=NULL,*B=NULL,*C=NULL;
    if (posix_memalign((void**)&A,64,szA*sizeof(double)) ||
        posix_memalign((void**)&B,64,szB*sizeof(double)) ||
        posix_memalign((void**)&C,64,szC*sizeof(double))) {
        fprintf(stderr,"posix_memalign failed\n"); return 2; }
    init_matrix(A, szA, 42u); init_matrix(B, szB, 24u); memset(C,0,szC*sizeof(double));

    const double alpha=1.0, beta=0.0;

    // ---- 预热 ----
    for (int i=0;i<warmup;++i)
        cblas_dgemm(CblasRowMajor,CblasNoTrans,CblasTrans,M,N,K,alpha,A,K,B,K,beta,C,N);

    // ---- 计时 ----
    double *times = (double*)malloc(sizeof(double)*repeat);
    if (!times) { free(A); free(B); free(C); fprintf(stderr, "alloc failed\n"); return 2; }
    for (int r=0;r<repeat;++r){
        memset(C,0,szC*sizeof(double));
        double t0s=now_sec();
        cblas_dgemm(CblasRowMajor,CblasNoTrans,CblasTrans,M,N,K,alpha,A,K,B,K,beta,C,N);
        double t1s=now_sec();
        times[r]=t1s-t0s;
    }
    double sum=0,tmin=1e99,tmax=0;
    for (int r=0;r<repeat;++r){ if(times[r]<tmin)tmin=times[r]; if(times[r]>tmax)tmax=times[r]; sum+=times[r]; }
    int denom = repeat>=3? repeat-2: repeat;
    double avg = (repeat>=3)? (sum-tmin-tmax)/denom : sum/repeat;

    double flops = 2.0*(double)M*(double)N*(double)K;
    double gflops = (avg>0.0)? (flops/1e9)/avg : INFINITY;
    printf("%d,%d,%d,%s,%s,%.6f,%.3f\n", M,N,K, env_threads, "dgemm", avg, gflops);

    free(times); free(A); free(B); free(C); return 0;
}

