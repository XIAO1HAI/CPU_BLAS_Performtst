#!/bin/bash
# OpenBLAS -> ✅
export LD_LIBRARY_PATH=/opt/OpenBLAS/lib/:$LD_LIBRARY_PATH
# armpl -> ✅
export LD_LIBRARY_PATH=/opt/arm/armpl_25.07_gcc/lib/:$LD_LIBRARY_PATH
# libxsmm -> ✅
export LD_LIBRARY_PATH=/opt/libxsmm/lib/:$LD_LIBRARY_PATH
# blis -> ✅ and blasfeo-> ❌
export LD_LIBRARY_PATH=/usr/local/lib/:$LD_LIBRARY_PATH
# LibShalom-> ❌
export LD_LIBRARY_PATH=/opt/LibShalom/lib/:$LD_LIBRARY_PATH

#export OMP_DYNAMIC=0

export OMP_NUM_THREADS=36
export OMP_DYNAMIC=false
export OMP_PROC_BIND=close
export OMP_PLACES=cores


export BLIS_AFFINITY="0-35"
taskset -c 0-35 ./yourprograme
#固定前 36 核：
export GOMP_CPU_AFFINITY="0-35"

#间隔绑核(例如只用偶数核)
export GOMP_CPU_AFFINITY="0-70:2"   # 从0到70步长2


#指定顺序(轮流分配给线程1..N)
export GOMP_CPU_AFFINITY="0 4 8 12 16 20 24 28 32 36 40 44 48 52 56 60 64 68 72 76 80

#gcc fp64_peak.c -O3 -ffast-math -march=armv8.2-a+simd -fopenmp -o fp64_peak