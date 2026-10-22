// bench_blasfeo_dgemm_nt.c -- NT path, OpenMP outer parallel, N-sliced, low-peak memory
#include "../../../../benchmark_args.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#include "blasfeo.h"
#include "blasfeo_d_aux.h"
#include "blasfeo_d_blas.h"

static inline double wall_time_sec(void) { return omp_get_wtime(); }

static void* aligned_alloc_bytes(size_t bytes) {
    void* p = NULL;
#if defined(_MSC_VER)
    p = _aligned_malloc(bytes, 64);
#else
    if (posix_memalign(&p, 64, bytes) != 0) p = NULL;
#endif
    return p;
}

static void fill_rand_colmajor(double* M, int rows, int cols, int ld) {
    for (int j = 0; j < cols; ++j)
        for (int i = 0; i < rows; ++i)
            M[i + (size_t)j*ld] = (double)rand() / RAND_MAX - 0.5;
}

static double gflops_dn(long long m, long long n, long long k, double sec) {
    const long double ops = 2.0L * (long double)m * (long double)n * (long double)k;
    return (double)(ops / (long double)sec / 1e9L);
}

static void* xmalloc(size_t nbytes, const char* tag) {
    void* p = aligned_alloc_bytes(nbytes);
    if (!p) {
        fprintf(stderr, "FATAL: alloc %s (%zu bytes) failed\n", tag, nbytes);
        exit(2);
    }
    return p;
}

int main(int argc, char** argv) {
    if (argc < 6) {
        fprintf(stderr, "Usage: %s M N K iters warmup [threads] [ns_slice]\n", argv[0]);
        return 1;
    }

    const int M      = bench_int(argv[1], 1);
    const int N      = bench_int(argv[2], 1);
    const int K      = bench_int(argv[3], 1);
    const int iters  = bench_int(argv[4], 1);
    const int warmup = bench_int(argv[5], 0);
    const int threads = (argc > 6) ? bench_int(argv[6], 1) : omp_get_max_threads();
    const int ns_user = (argc > 7) ? bench_int(argv[7], 1) : 2048;   // N 方向切片大小（默认 2048）

    if (ns_user <= 0) { fprintf(stderr, "ns_slice must be > 0\n"); return 1; }

    bench_elements(M, K); bench_elements(N, K); bench_elements(M, N);
    omp_set_num_threads(threads);

    const double alpha = 1.0, beta = 0.0;

    // ===== A: 全量面板（M x K） =====
    size_t memA = blasfeo_memsize_dmat(M, K);
    void* bufA = xmalloc(memA, "sA");
    struct blasfeo_dmat sA;
    blasfeo_create_dmat(M, K, &sA, bufA);

    // 仅为 pack A 临时分配列主序 A，完成后释放
    {
        const int lda = M;
        size_t bytesA = sizeof(double) * (size_t)lda * (size_t)K;
        double* A = (double*)xmalloc(bytesA, "A(colmajor)");
        srand(0);
        fill_rand_colmajor(A, M, K, lda);
        blasfeo_pack_dmat(M, K, A, lda, &sA, 0, 0);
        free(A);
    }

    // ===== NT panels: B_slice (ns x K), C_slice (M x ns) =====

    // 为校验用的一列临时缓冲（O(M)）
    double* col = (double*)xmalloc(sizeof(double) * (size_t)M, "colbuf");

    double checksum = 0.0;

    // 预热
    for (int w = 0; w < warmup; ++w) {
        for (int j0 = 0; j0 < N; j0 += ns_user) {
            const int ns = (j0 + ns_user <= N) ? ns_user : (N - j0);

            // 为 B_slice/C_slice 显式分配面板缓冲
            size_t memB = blasfeo_memsize_dmat(ns, K);
            size_t memC = blasfeo_memsize_dmat(M, ns);
            void* bufB = xmalloc(memB, "sB_slice");
            void* bufC = xmalloc(memC, "sC_slice");
            struct blasfeo_dmat sB, sC;
            blasfeo_create_dmat(ns, K, &sB, bufB);
            blasfeo_create_dmat(M, ns, &sC, bufC);

            // Pack the column-major ns x K panel consumed as B_slice^T.
            {
                const int ldb = ns;
                size_t bytesB = sizeof(double) * (size_t)ldb * (size_t)K;
                double* Bsl = (double*)xmalloc(bytesB, "B_slice(colmajor)");
                // 生成 B 的随机数：为了可复现，用 (j0) 作为种子偏移
                srand(1 + j0);
                fill_rand_colmajor(Bsl, ns, K, ldb);
                blasfeo_pack_dmat(ns, K, Bsl, ldb, &sB, 0, 0);
                free(Bsl);
            }

            // sC 清零（beta=0，非必须，但更明确）
            blasfeo_dgese(M, ns, 0.0, &sC, 0, 0);

            // 外层并行：沿 M 切
#pragma omp parallel
            {
                int tid = omp_get_thread_num();
                int nth = omp_get_num_threads();
                int i0  = (long long)M * tid / nth;
                int i1  = (long long)M * (tid + 1) / nth;
                int mb  = i1 - i0;
                if (mb > 0) {
                    // D = beta*C + alpha*A*B^T, 这里我们令 D 与 C 同块（原地）
                    blasfeo_dgemm_nt(mb, ns, K,
                                     alpha,
                                     &sA, i0, 0,
                                     &sB, 0,  0,
                                     beta,
                                     &sC, i0, 0,   // C
                                     &sC, i0, 0);  // D
                }
            }

            // 预热阶段不做 checksum，直接释放切片
            free(bufB);
            free(bufC);
        }
    }

    // 计时
    double t0 = wall_time_sec();
    for (int it = 0; it < iters; ++it) {
        for (int j0 = 0; j0 < N; j0 += ns_user) {
            const int ns = (j0 + ns_user <= N) ? ns_user : (N - j0);

            size_t memB = blasfeo_memsize_dmat(ns, K);
            size_t memC = blasfeo_memsize_dmat(M, ns);
            void* bufB = xmalloc(memB, "sB_slice");
            void* bufC = xmalloc(memC, "sC_slice");
            struct blasfeo_dmat sB, sC;
            blasfeo_create_dmat(ns, K, &sB, bufB);
            blasfeo_create_dmat(M, ns, &sC, bufC);

            // pack 当前 B_slice
            {
                const int ldb = ns;
                size_t bytesB = sizeof(double) * (size_t)ldb * (size_t)K;
                double* Bsl = (double*)xmalloc(bytesB, "B_slice(colmajor)");
                srand(1 + j0);
                fill_rand_colmajor(Bsl, ns, K, ldb);
                blasfeo_pack_dmat(ns, K, Bsl, ldb, &sB, 0, 0);
                free(Bsl);
            }

            // C_slice 清零
            blasfeo_dgese(M, ns, 0.0, &sC, 0, 0);

            // 并行 GEMM（沿 M 切）
#pragma omp parallel
            {
                int tid = omp_get_thread_num();
                int nth = omp_get_num_threads();
                int i0  = (long long)M * tid / nth;
                int i1  = (long long)M * (tid + 1) / nth;
                int mb  = i1 - i0;
                if (mb > 0) {
                    blasfeo_dgemm_nt(mb, ns, K,
                                     alpha,
                                     &sA, i0, 0,
                                     &sB, 0,  0,
                                     beta,
                                     &sC, i0, 0,
                                     &sC, i0, 0);
                }
            }

            // 轻量 checksum：采样该切片的一列（例如切片内最后一列）
            int jpick = ns - 1;
            blasfeo_unpack_dmat(M, 1, &sC, 0, jpick, col, M);
            checksum += col[M - 1];

            free(bufB);
            free(bufC);
        }
    }
    double t1 = wall_time_sec();

    double avg = (t1 - t0) / (double)iters;
    double gf  = gflops_dn(M, N, K, avg);

    printf("BLASFEO dgemm NT | M=%d N=%d K=%d | threads=%d | ns=%d | time=%.6f s | %.2f GFLOPS | checksum=%.6e\n",
           M, N, K, threads, ns_user, avg, gf, checksum);

    free(col);
    free(bufA);
    return 0;
}
