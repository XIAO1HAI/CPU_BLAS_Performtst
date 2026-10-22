#!/usr/bin/env bash
# Source this file before running a benchmark; all values remain overridable.
export OPENBLAS_ROOT="${OPENBLAS_ROOT:-/opt/OpenBLAS}"
export ARMPL_DIR="${ARMPL_DIR:-/opt/arm/armpl_25.07_gcc}"
export BLIS_HOME="${BLIS_HOME:-/usr/local}"
export LIBXSMM_ROOT="${LIBXSMM_ROOT:-/opt/libxsmm}"
export LD_LIBRARY_PATH="${OPENBLAS_ROOT}/lib:${ARMPL_DIR}/lib:${BLIS_HOME}/lib:${LIBXSMM_ROOT}/lib:${LD_LIBRARY_PATH:-}"
export OMP_NUM_THREADS="${OMP_NUM_THREADS:-36}"
export OMP_DYNAMIC=false
export OMP_PROC_BIND="${OMP_PROC_BIND:-close}"
export OMP_PLACES="${OMP_PLACES:-cores}"
export OPENBLAS_NUM_THREADS="${OPENBLAS_NUM_THREADS:-$OMP_NUM_THREADS}"
export BLIS_NUM_THREADS="${BLIS_NUM_THREADS:-$OMP_NUM_THREADS}"
