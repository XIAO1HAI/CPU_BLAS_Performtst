// fp64_peak_sve.c  —— SVE(FP64) 合成峰值
#include "../benchmark_args.h"
#include <arm_sve.h>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

static inline double now_sec(){
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(int argc, char** argv){
    uint64_t iters = (argc>1)? bench_u64(argv[1], 1, UINT64_MAX) : (uint64_t)6e8; // SVE更宽，迭代少些
    double t0 = now_sec(), sink = 0.0;
    int T = 0;

    #pragma omp parallel reduction(+:sink)
    {
        #pragma omp single
        T = omp_get_num_threads();
        svbool_t pg = svptrue_b64();
        svfloat64_t a0 = svdup_f64(1.001), b0 = svdup_f64(1.0001);
        svfloat64_t a1 = svdup_f64(0.999),  b1 = svdup_f64(0.9999);

        svfloat64_t c0 = svdup_f64(0.0),  c1  = c0,  c2  = c0,  c3  = c0;
        svfloat64_t c4 = c0,              c5  = c0,  c6  = c0,  c7  = c0;
        svfloat64_t c8 = c0,              c9  = c0,  c10 = c0,  c11 = c0;
        svfloat64_t c12= c0,              c13 = c0,  c14 = c0,  c15 = c0;

        // 16 independent FMA instructions per iteration.
        for(uint64_t i=0;i<iters;++i){
            c0  = svmla_f64_x(pg, c0,  a0, b0);  c1  = svmla_f64_x(pg, c1,  a1, b1);
            c2  = svmla_f64_x(pg, c2,  a0, b0);  c3  = svmla_f64_x(pg, c3,  a1, b1);
            c4  = svmla_f64_x(pg, c4,  a0, b0);  c5  = svmla_f64_x(pg, c5,  a1, b1);
            c6  = svmla_f64_x(pg, c6,  a0, b0);  c7  = svmla_f64_x(pg, c7,  a1, b1);
            c8  = svmla_f64_x(pg, c8,  a0, b0);  c9  = svmla_f64_x(pg, c9,  a1, b1);
            c10 = svmla_f64_x(pg, c10, a0, b0);  c11 = svmla_f64_x(pg, c11, a1, b1);
            c12 = svmla_f64_x(pg, c12, a0, b0);  c13 = svmla_f64_x(pg, c13, a1, b1);
            c14 = svmla_f64_x(pg, c14, a0, b0);  c15 = svmla_f64_x(pg, c15, a1, b1);
            __asm__ volatile("" : "+w"(c0), "+w"(c1), "+w"(c2), "+w"(c3), "+w"(c4), "+w"(c5), "+w"(c6), "+w"(c7));
            __asm__ volatile("" : "+w"(c8), "+w"(c9), "+w"(c10), "+w"(c11), "+w"(c12), "+w"(c13), "+w"(c14), "+w"(c15));
        }

        // 防优化：简单归约
        svfloat64_t s0 = svadd_f64_x(pg, svadd_f64_x(pg,c0,c1),  svadd_f64_x(pg,c2,c3));
        svfloat64_t s1 = svadd_f64_x(pg, svadd_f64_x(pg,c4,c5),  svadd_f64_x(pg,c6,c7));
        svfloat64_t s2 = svadd_f64_x(pg, svadd_f64_x(pg,c8,c9),  svadd_f64_x(pg,c10,c11));
        svfloat64_t s3 = svadd_f64_x(pg, svadd_f64_x(pg,c12,c13),svadd_f64_x(pg,c14,c15));
        svfloat64_t all = svadd_f64_x(pg, svadd_f64_x(pg,s0,s1), svadd_f64_x(pg,s2,s3));
        double tmp[svcntd()]; svst1_f64(pg, tmp, all);
        sink += tmp[0];
    }

    double secs = now_sec() - t0;
    int lanes = svcntd();                 // VL=32B 时 lanes=4（双精度）
    int fma_per_iter = 16;                // 16 FMA instructions per iteration
    double gflops = (double)iters * fma_per_iter * lanes * 2.0 * T / 1e9 / secs;

    printf("SVE FP64 peak: VL=%dB lanes=%d, threads=%d, time=%.3f s, GFLOPS=%.1f (sink=%.3e)\n",
           lanes*8, lanes, T, secs, gflops, sink);
    return 0;
}
