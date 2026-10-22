// dgemm_bench_cblas.c
// 用 CBLAS 接口测试 DGEMM-NT：C = A * B^T
// 适用于 OpenBLAS / BLIS / ARMPL（链接不同库即可）
//
// Usage:
//   ./dgemm_bench_<lib> --m 4096 --n 4096 --k 4096 --layout row --transA N --transB T --repeats 5 --warmup 1
//
// 线程控制：通过环境变量（如 OPENBLAS_NUM_THREADS / BLIS_NUM_THREADS / OMP_NUM_THREADS）
// 亲和性：建议搭配 numactl --cpunodebind=0 --membind=0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cblas.h>

static double wall_time_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static void* xaligned_alloc(size_t nbytes, size_t align) {
    void* p = NULL;
    if (posix_memalign(&p, align, nbytes) != 0) return NULL;
    return p;
}

int main(int argc, char** argv) {
    int m = 1024, n = 1024, k = 1024;
    int repeats = 5, warmup = 1;
    double alpha = 1.0, beta = 0.0;

    CBLAS_LAYOUT layout = CblasRowMajor;
    CBLAS_TRANSPOSE ta = CblasNoTrans; // N
    CBLAS_TRANSPOSE tb = CblasTrans;   // T (NT 形态)

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--m")) m = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--n")) n = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--k")) k = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--repeats")) repeats = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--warmup")) warmup = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--alpha")) alpha = atof(argv[++i]);
        else if (!strcmp(argv[i], "--beta"))  beta  = atof(argv[++i]);
        else if (!strcmp(argv[i], "--layout")) {
            const char* s = argv[++i];
            if (!strcasecmp(s, "row")) layout = CblasRowMajor;
            else if (!strcasecmp(s, "col")) layout = CblasColMajor;
            else { fprintf(stderr, "Unknown layout %s\n", s); return 1; }
        }
        else if (!strcmp(argv[i], "--transA")) {
            char c = argv[++i][0];
            ta = (c=='N'||c=='n')? CblasNoTrans : CblasTrans;
        }
        else if (!strcmp(argv[i], "--transB")) {
            char c = argv[++i][0];
            tb = (c=='N'||c=='n')? CblasNoTrans : CblasTrans;
        }
        else {
            fprintf(stderr, "Unknown arg: %s\n", argv[i]);
            return 1;
        }
    }

    int a_rows = (ta==CblasNoTrans)? m : k;
    int a_cols = (ta==CblasNoTrans)? k : m;
    int b_rows = (tb==CblasNoTrans)? k : n;
    int b_cols = (tb==CblasNoTrans)? n : k;

    int lda, ldb, ldc;
    if (layout == CblasRowMajor) {
        lda = a_cols;  // row-major: ld = 列数
        ldb = b_cols;
        ldc = n;
    } else {
        lda = a_rows;  // col-major: ld = 行数
        ldb = b_rows;
        ldc = m;
    }

    size_t align = 64;
    double* A = (double*)xaligned_alloc((size_t)a_rows*a_cols*sizeof(double), align);
    double* B = (double*)xaligned_alloc((size_t)b_rows*b_cols*sizeof(double), align);
    double* C = (double*)xaligned_alloc((size_t)m*n*sizeof(double), align);
    if (!A || !B || !C) { fprintf(stderr, "alloc failed\n"); return 1; }

    // 初始化
    for (size_t i=0;i<(size_t)a_rows*a_cols;i++) A[i] = (double)((i%13)-6);
    for (size_t i=0;i<(size_t)b_rows*b_cols;i++) B[i] = (double)((i%17)-8);
    for (size_t i=0;i<(size_t)m*n;i++) C[i] = 0.0;

    // 预热
    for (int t=0; t<warmup; ++t) {
        cblas_dgemm(layout, ta, tb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc);
    }

    // 计时（取最短）
    double best = 1e100;
    for (int r=0; r<repeats; ++r) {
        if (beta == 0.0) memset(C, 0, (size_t)m*n*sizeof(double));
        double t0 = wall_time_sec();
        cblas_dgemm(layout, ta, tb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc);
        double t1 = wall_time_sec();
        if (t1 - t0 < best) best = t1 - t0;
    }
    double gflops = (2.0 * (double)m * (double)n * (double)k) / best / 1e9;

    printf("m=%d n=%d k=%d layout=%s transA=%c transB=%c time=%.6f s  GFLOP/s=%.2f\n",
           m, n, k,
           (layout==CblasRowMajor? "row":"col"),
           (ta==CblasNoTrans? 'N':'T'),
           (tb==CblasNoTrans? 'N':'T'),
           best, gflops);

    free(A); free(B); free(C);
    return 0;
}
