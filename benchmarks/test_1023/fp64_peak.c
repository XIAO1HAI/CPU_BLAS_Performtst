// fp64_peak.c : AArch64 NEON 合成峰值(FP64)
#include <arm_neon.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <omp.h>

static inline double now_sec() {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(int argc, char** argv) {
    uint64_t iters = (argc > 1) ? strtoull(argv[1], NULL, 10) : (uint64_t)24e8;

    float64x2_t a0 = vdupq_n_f64(1.001), b0 = vdupq_n_f64(1.0001);
    float64x2_t a1 = vdupq_n_f64(0.999), b1 = vdupq_n_f64(0.9999);

    double t0 = now_sec();
    double sink = 0.0;

    #pragma omp parallel reduction(+:sink)
    {
        float64x2_t c0 = vdupq_n_f64(0.0),  c1 = c0,  c2 = c0,  c3 = c0;
        float64x2_t c4 = c0,  c5 = c0,  c6 = c0,  c7 = c0;
        float64x2_t c8 = c0,  c9 = c0, c10 = c0, c11 = c0;
        float64x2_t c12= c0, c13 = c0, c14 = c0, c15 = c0;

        // 每轮 32 条 vfmaq_f64；每条 4 FLOPs => 每轮 128 FLOPs/线程
        for (uint64_t i = 0; i < iters; ++i) {
            c0  = vfmaq_f64(c0,  a0, b0);  c1  = vfmaq_f64(c1,  a1, b1);
            c2  = vfmaq_f64(c2,  a0, b0);  c3  = vfmaq_f64(c3,  a1, b1);
            c4  = vfmaq_f64(c4,  a0, b0);  c5  = vfmaq_f64(c5,  a1, b1);
            c6  = vfmaq_f64(c6,  a0, b0);  c7  = vfmaq_f64(c7,  a1, b1);
            c8  = vfmaq_f64(c8,  a0, b0);  c9  = vfmaq_f64(c9,  a1, b1);
            c10 = vfmaq_f64(c10, a0, b0);  c11 = vfmaq_f64(c11, a1, b1);
            c12 = vfmaq_f64(c12, a0, b0);  c13 = vfmaq_f64(c13, a1, b1);
            c14 = vfmaq_f64(c14, a0, b0);  c15 = vfmaq_f64(c15, a1, b1);
        }

        float64x2_t sum0 = vaddq_f64(vaddq_f64(c0, c1),  vaddq_f64(c2, c3));
        float64x2_t sum1 = vaddq_f64(vaddq_f64(c4, c5),  vaddq_f64(c6, c7));
        float64x2_t sum2 = vaddq_f64(vaddq_f64(c8, c9),  vaddq_f64(c10,c11));
        float64x2_t sum3 = vaddq_f64(vaddq_f64(c12,c13), vaddq_f64(c14,c15));
        float64x2_t all  = vaddq_f64(vaddq_f64(sum0,sum1), vaddq_f64(sum2,sum3));
        double tmp[2]; vst1q_f64(tmp, all);
        sink += tmp[0] + tmp[1];
    }

    double t1 = now_sec();
    double secs = t1 - t0;
    int T = omp_get_max_threads();

    double flops_per_iter_per_thr = 32.0 * 4.0; // 32 条 × 4 FLOPs
    double gflops = (flops_per_iter_per_thr * (double)iters * (double)T) / 1e9 / secs;

    printf("FP64 peak synthetic: threads=%d, iters=%llu, time=%.3f s, GFLOPS=%.1f (sink=%g)\n",
           T, (unsigned long long)iters, secs, gflops, sink);
    return 0;
}
