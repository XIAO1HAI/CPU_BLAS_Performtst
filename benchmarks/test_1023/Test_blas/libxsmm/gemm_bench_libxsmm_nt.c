#include "../../../benchmark_args.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <libxsmm.h>

/*
 * 用法:
 *   ./gemm_bench_libxsmm_nt M N K REP [VERIFY]
 * 语义:
 *   计算 C = A * B^T  (NT)
 *   A: m x k,  lda=m
 *   B: n x k,  ldb=n  (存储为“未转置”，调用时 transb='T')
 *   C: m x n,  ldc=m
 * 线程数用 OMP_NUM_THREADS 控制
 */

static void fill_rand(double* a, size_t n) {
  for (size_t i = 0; i < n; ++i) a[i] = (double)rand()/RAND_MAX - 0.5;
}

int main(int argc, char** argv) {
  if (argc < 5) {
    fprintf(stderr, "Usage: %s M N K REP [VERIFY]\n", argv[0]);
    return 1;
  }
  const libxsmm_blasint m   = (libxsmm_blasint)bench_int(argv[1], 1);
  const libxsmm_blasint n   = (libxsmm_blasint)bench_int(argv[2], 1);
  const libxsmm_blasint k   = (libxsmm_blasint)bench_int(argv[3], 1);
  const int               rep = bench_int(argv[4], 1);
  const int            verify = (argc > 5 ? bench_int(argv[5], 0) : 0);

  const libxsmm_blasint lda = m;
  const libxsmm_blasint ldb = n;   /* B 为 n x k（列主序）；以 transb='T' 使用 */
  const libxsmm_blasint ldc = m;

  const size_t szA = bench_elements(lda, k);
  const size_t szB = bench_elements(ldb, k);
  const size_t szC = bench_elements(ldc, n);

  double *A = (double*)libxsmm_aligned_malloc(szA * sizeof(double), 64);
  double *B = (double*)libxsmm_aligned_malloc(szB * sizeof(double), 64);
  double *C = (double*)libxsmm_aligned_malloc(szC * sizeof(double), 64);
  double *Cref = verify ? (double*)libxsmm_aligned_malloc(szC * sizeof(double), 64) : NULL;

  if (!A || !B || !C || (verify && !Cref)) { fprintf(stderr, "alloc failed\n"); return 2; }

  srand(777);
  fill_rand(A, szA);
  fill_rand(B, szB);
  memset(C, 0, szC * sizeof(double));
  if (Cref) memset(Cref, 0, szC * sizeof(double));

  libxsmm_init();

  const char transa = 'N', transb = 'T';
  const double alpha = 1.0, beta = 0.0;

  /* 更细粒度的 2D 分块并行 */
  const libxsmm_blasint MB = 128; /* 行块大小 */
  const libxsmm_blasint NB =  64; /* 列块大小 */

  /* 避免 OMP 动态缩放线程 */
  omp_set_dynamic(0);

  const double flop = 2.0 * (double)m * (double)n * (double)k;
  const double t0 = omp_get_wtime();

  for (int r = 0; r < rep; ++r) {
    #pragma omp parallel for collapse(2) schedule(static)
    for (libxsmm_blasint i0 = 0; i0 < m; i0 += MB) {
      for (libxsmm_blasint j0 = 0; j0 < n; j0 += NB) {
        const libxsmm_blasint ib = (i0 + MB <= m ? MB : (m - i0));
        const libxsmm_blasint jb = (j0 + NB <= n ? NB : (n - j0));
        /* A 子块起点：A(i0,0) => A + i0
           B 子块起点：B(j0,0) => B + j0  （仍按 transb='T' 消费）
           C 子块起点：C(i0,j0) => C + i0 + (size_t)j0 * ldc
         */
        libxsmm_dgemm(&transa, &transb,
                      &ib, &jb, &k,
                      &alpha,
                      A + i0, &lda,
                      B + j0, &ldb,
                      &beta,
                      C + i0 + (size_t)j0 * ldc, &ldc);
      }
    }
  }

  const double t1 = omp_get_wtime();

  const double secs = t1 - t0;
  const double gflops = (flop * rep) / secs / 1e9;
  printf("LIBXSMM dgemm NT  m=%d n=%d k=%d rep=%d  time=%.6f s  GFLOPS=%.2f\n",
         (int)m, (int)n, (int)k, rep, secs, gflops);

  int status = 0;
  if (verify) {
    /* 朴素参考：Cref = A * B^T */
    for (libxsmm_blasint j = 0; j < n; ++j) {
      for (libxsmm_blasint i = 0; i < m; ++i) {
        double acc = 0.0;
        for (libxsmm_blasint t = 0; t < k; ++t) {
          acc += A[i + (size_t)t * lda] * B[j + (size_t)t * ldb];
        }
        Cref[i + (size_t)j * ldc] = acc;
      }
    }
    double maxerr = 0.0;
    for (size_t q = 0; q < szC; ++q) {
      double e = fabs(C[q] - Cref[q]);
      if (!isfinite(C[q]) || !isfinite(Cref[q]) || e > 1e-10 * (1.0 + fabs(Cref[q]))) status = 3;
      if (e > maxerr) maxerr = e;
    }
    printf("Max |C-Cref| = %.3e\n", maxerr);
  }

  libxsmm_finalize();
  libxsmm_free(A); libxsmm_free(B); libxsmm_free(C);
  if (Cref) libxsmm_free(Cref);
  return status;
}

