// dgemm_nt_bench_blis.c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <string.h>
// #include <cblas.h>   // 由 BLIS 提供的 CBLAS 兼容层（安装后自带 cblas.h）
#include "blis/cblas.h"

// 高精度计时（Linux clock_gettime）
static double now_sec() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + 1e-9 * ts.tv_nsec;
}

// 随机初始化
static void fill_random(double* x, size_t n) {
    for (size_t i = 0; i < n; ++i) x[i] = (double)rand() / RAND_MAX - 0.5;
}

// 计算 GFLOPS = 2*M*N*K / 1e9 / time
static inline double gflops(long M, long N, long K, double sec) {
    return (2.0 * (double)M * (double)N * (double)K) / 1e9 / sec;
}

int main(int argc, char** argv) {
    if (argc < 7) {
        fprintf(stderr, "Usage: %s M N K iters warmup_iters threads\n", argv[0]);
        return 1;
    }
    const int M = atoi(argv[1]);
    const int N = atoi(argv[2]);
    const int K = atoi(argv[3]);
    const int iters = atoi(argv[4]);
    const int wup   = atoi(argv[5]);
    const int threads = atoi(argv[6]); // BLIS_NUM_THREADS

    // 设置 BLIS 线程数（CBLAS 也走 BLIS 内核）
    setenv("BLIS_NUM_THREADS", argv[6], 1);
    // 也可用子策略控制（可选）：
    // setenv("BLIS_JC_NT", argv[6], 1);

    const int lda = M;  // 列主序（CBLAS 假定列主序）；A: M x K
    const int ldb = N;  // B^T 的存储按 B 原矩阵（K x N）列主序，因此传给 dgemm 用 opB = CblasTrans
    const int ldc = M;  // C: M x N

    // 分配矩阵（列主序）
    double* A = (double*) aligned_alloc(64, (size_t)M * K * sizeof(double));
    double* B = (double*) aligned_alloc(64, (size_t)K * N * sizeof(double));
    double* C = (double*) aligned_alloc(64, (size_t)M * N * sizeof(double));
    if (!A || !B || !C) { fprintf(stderr, "alloc failed\n"); return 2; }

    srand(0);
    fill_random(A, (size_t)M*K);
    fill_random(B, (size_t)K*N);
    memset(C, 0, (size_t)M*N*sizeof(double));

    const double alpha = 1.0, beta = 0.0;

    // 预热
    for (int i = 0; i < wup; ++i) {
        bli_dgemm(CblasColMajor, CblasNoTrans, CblasTrans,  // NT
                    M, N, K, alpha, A, lda, B, ldb, beta, C, ldc);
    }

    double t0 = now_sec();
    for (int i = 0; i < iters; ++i) {
        bli_dgemm(CblasColMajor, CblasNoTrans, CblasTrans,  // NT
                    M, N, K, alpha, A, lda, B, ldb, beta, C, ldc);
    }
    double t1 = now_sec();
    double avg = (t1 - t0) / (double)iters;

    printf("BLIS DGEMM NT  M=%d N=%d K=%d  thr=%d  time=%.6f s  GFLOPS=%.2f\n",
           M, N, K, threads, avg, gflops(M,N,K,avg));

    free(A); free(B); free(C);
    return 0;
}

