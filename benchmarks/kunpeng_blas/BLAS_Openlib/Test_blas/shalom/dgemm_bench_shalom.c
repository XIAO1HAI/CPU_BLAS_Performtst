#include "../../../../benchmark_args.h"
#include <string.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include "LibShalom.h"

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

// Preserve the native NN path for supported tiles; finish uncovered edge elements.
static void shalom_nn(double *C, double *A, double *B, int M, int N, int K) {
    int native_rows = M % 7 == 6 ? M : M - M % 7;
    if (K < 4 || N < 6) native_rows = 0;
    if (native_rows) LibShalom_dgemm(NoTrans, NoTrans, C, A, B, native_rows, N, K);
    int native_columns = N - N % 6;
    for (int i = 0; i < M; ++i) {
        for (int j = i < native_rows ? native_columns : 0; j < N; ++j) {
            double value = 0.0;
            for (int k = 0; k < K; ++k) value += A[(size_t)i*K+k]*B[(size_t)k*N+j];
            C[(size_t)i*N+j] = value;
        }
    }
}

int main(int argc, char **argv) {
    int M=384, N=384, K=384, repeats=100, warmup=5, verify=0;
    const char *env_threads=getenv("OMP_NUM_THREADS");
    int threads=env_threads ? bench_int(env_threads, 1) : omp_get_max_threads();
    for (int i=1; i<argc; ++i) {
        if (!strcmp(argv[i], "--verify")) { verify=1; continue; }
        if (i+1>=argc) { fprintf(stderr, "Missing value for %s\n", argv[i]); return 1; }
        const char *key=argv[i], *value=argv[++i];
        if (!strcmp(key,"--m")) M=bench_int(value,1);
        else if (!strcmp(key,"--n")) N=bench_int(value,1);
        else if (!strcmp(key,"--k")) K=bench_int(value,1);
        else if (!strcmp(key,"--repeats")) repeats=bench_int(value,1);
        else if (!strcmp(key,"--warmup")) warmup=bench_int(value,0);
        else if (!strcmp(key,"--threads")) threads=bench_int(value,1);
        else if ((!strcmp(key,"--layout") && !strcmp(value,"row")) ||
                 ((!strcmp(key,"--transA") || !strcmp(key,"--transB")) && !strcmp(value,"N"))) {}
        else { fprintf(stderr,"LibShalom benchmark supports row-major NN; invalid option: %s %s\n",key,value); return 1; }
    }
    size_t a_count=bench_elements(M,K), b_count=bench_elements(K,N), c_count=bench_elements(M,N);
    double *A=(double*)bench_aligned_alloc(a_count*sizeof(double));
    double *B=(double*)bench_aligned_alloc(b_count*sizeof(double));
    double *C=(double*)bench_aligned_alloc(c_count*sizeof(double));
    if (!A || !B || !C) { free(A); free(B); free(C); fprintf(stderr,"alloc failed\n"); return 2; }
    for (size_t i=0;i<a_count;++i) A[i]=(double)((int)(i%13)-6)/13.0;
    for (size_t i=0;i<b_count;++i) B[i]=(double)((int)(i%17)-8)/17.0;
    memset(C,0,c_count*sizeof(double));
    LibShalom_set_thread_nums(threads);
    for (int i=0;i<warmup;++i) shalom_nn(C,A,B,M,N,K);
    double t0=now_sec();
    for (int i=0;i<repeats;++i) shalom_nn(C,A,B,M,N,K);
    double seconds=(now_sec()-t0)/repeats;
    printf("m=%d n=%d k=%d transA=N transB=N threads_requested=%d time=%.9f s GFLOP/s=%.2f\n",
           M,N,K,threads,seconds,2.0*M*N*K/seconds/1e9);
    int status=0;
    if (verify) {
        double maxerr=0.0;
        for (int i=0;i<M;++i) for (int j=0;j<N;++j) {
            double reference=0.0;
            for (int k=0;k<K;++k) reference+=A[(size_t)i*K+k]*B[(size_t)k*N+j];
            double error=fabs(C[(size_t)i*N+j]-reference);
            if (!isfinite(C[(size_t)i*N+j]) || error>1e-10*(1.0+fabs(reference))) status=3;
            if (error>maxerr) maxerr=error;
        }
        printf("Max |C-Cref| = %.3e\n",maxerr);
    }
    free(A); free(B); free(C);
    return status;
}
