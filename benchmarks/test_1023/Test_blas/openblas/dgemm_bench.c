// dgemm_bench.c
// Build: link OpenBLAS or BLIS providing cblas_dgemm
// Usage example:
//   ./dgemm_bench --m 4096 --n 4096 --k 4096 --layout row --transA N --transB T --repeats 5 --warmup 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cblas.h>

// void LibShalom_set_thread_nums(int num);
// int LibShalom_dgemm(int transa, int transb,
//                     double *C, double *A, double *B,
//                     long M, long N, long K);
static double wall_time_sec() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void* xaligned_alloc(size_t nbytes, size_t align) {
    void* p = NULL;
    if (posix_memalign(&p, align, nbytes) != 0) return NULL;
    return p;
}

int main(int argc, char** argv) {
    // defaults
    int m=2688, n=77440, k=1920;
    int repeats=100, warmup=5;
    CBLAS_LAYOUT layout = CblasRowMajor;       // RowMajor 推荐（C更方便）
    CBLAS_TRANSPOSE ta = CblasNoTrans;         // NT
    CBLAS_TRANSPOSE tb = CblasTrans;
    double alpha = 1.0, beta = 0.0;

    // parse args
    for (int i=1; i<argc; ++i) {
        if (!strcmp(argv[i], "--m")) m = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--n")) n = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--k")) k = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--repeats")) repeats = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--warmup")) warmup = atoi(argv[++i]);
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
        else if (!strcmp(argv[i], "--alpha")) alpha = atof(argv[++i]);
        else if (!strcmp(argv[i], "--beta"))  beta  = atof(argv[++i]);
        else {
            fprintf(stderr, "Unknown arg: %s\n", argv[i]); return 1;
        }
    }

    // derive leading dimensions & buffer shapes under RowMajor
    int lda, ldb, ldc;
    int a_rows = (ta==CblasNoTrans)? m : k;
    int a_cols = (ta==CblasNoTrans)? k : m;
    int b_rows = (tb==CblasNoTrans)? k : n;
    int b_cols = (tb==CblasNoTrans)? n : k;

    if (layout == CblasRowMajor) {
        lda = a_cols; // row-major: ld = number of columns
        ldb = b_cols;
        ldc = n;
    } else {
        // Column-major: ld = number of rows
        lda = a_rows;
        ldb = b_rows;
        ldc = m;
    }

    size_t align = 64;
    double* A = (double*)xaligned_alloc((size_t)a_rows*a_cols*sizeof(double), align);
    double* B = (double*)xaligned_alloc((size_t)b_rows*b_cols*sizeof(double), align);
    double* C = (double*)xaligned_alloc((size_t)m*n*sizeof(double), align);
    if (!A || !B || !C) { fprintf(stderr, "alloc failed\n"); return 1; }

    // init
    for (size_t i=0;i<(size_t)a_rows*a_cols;i++) A[i] = (double)((i%13)-6);
    for (size_t i=0;i<(size_t)b_rows*b_cols;i++) B[i] = (double)((i%17)-8);
    for (size_t i=0;i<(size_t)m*n;i++) C[i] = 0.0;

    // warmup
    for (int t=0;t<warmup;t++) {
        cblas_dgemm(layout, ta, tb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc);
        // LibShalom_dgemm(NoTrans, Trans, C, A, B, M, N, K);
    }

    // measure
    double best = 1e100;
    double sum = 0;
    for (int r=0;r<repeats;r++) {
        // reset C if beta=0 to avoid accumulation影响
        if (beta==0.0) memset(C, 0, (size_t)m*n*sizeof(double));
        double t0 = wall_time_sec();
        cblas_dgemm(layout, ta, tb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc);
        double t1 = wall_time_sec();
        // if (t1 - t0 < best) best = t1 - t0;
        sum += (t1 - t0);
    }
    double avg = sum / repeats;

    double gflops = (2.0 * (double)m * (double)n * (double)k) / avg / 1e9;
    printf("m=%d n=%d k=%d layout=%s transA=%c transB=%c time=%.6f s  GFLOP/s=%.2f\n",
           m, n, k,
           (layout==CblasRowMajor? "row":"col"),
           (ta==CblasNoTrans? 'N':'T'),
           (tb==CblasNoTrans? 'N':'T'),
           avg, gflops);

    free(A); free(B); free(C);
    return 0;
}


