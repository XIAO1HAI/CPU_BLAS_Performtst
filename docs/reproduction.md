# Build and run

## Environment

The benchmark drivers target Linux / AArch64 with GCC, G++, Make, Bash 4+, and OpenMP. The NEON probe uses AArch64 FP64 SIMD; the SVE probe additionally requires SVE hardware support. Hardware checks use the servers' existing GCC 10.3.1 and installed or previously built BLAS libraries.

| Source label | Original workspace | Captured environment |
| --- | --- | --- |
| `kunpeng_blas` | `/home/workspace/Yhaibiao/KunPeng_BLAS` | Kunpeng 920, 256 logical CPUs, 4 sockets, 8 NUMA nodes |
| `test_1023` | `/home/workspace/Yhaibiao/test-1023` | Kunpeng 920 7285Z, 640 logical CPUs / 320 physical cores, 8 NUMA nodes |

The captures in [environments](environments/) describe the machines at import time. Historical result collections retain the platform labels from their original source folders.

## Dependencies

`dependencies/<source>/sources.json` records the source URL and full commit for each library workspace. Typical revisions are:

| Library | Revision |
| --- | --- |
| OpenBLAS | `aef36a3ff00842b09c0ef706c97d8fda10ef9584` |
| BLIS | `daa6e41ff2a6798bb4b5fa646a35f748bb8b0eae` |
| LIBXSMM | `10b7dc82b3c46157e76eb40e4e959555f895b24d` |
| LibShalom | `9b16391d68370cca3dde53dcb687576336ecfc64` |
| BLASFEO | `4e0d2f317174aae8fdc15059fe404fc9155bd5f3` |

For example:

```bash
mkdir -p third_party
git clone https://gitee.com/xiao--hai/OpenBLAS.git third_party/OpenBLAS
git -C third_party/OpenBLAS checkout --detach aef36a3ff00842b09c0ef706c97d8fda10ef9584
```

Build each fixed revision according to its upstream instructions. Install CBLAS headers and enable the intended threading implementation. ARMPL 25.07 GCC packages are obtained separately from Arm and are governed by Arm's license.

Intentional source annotations and LibShalom experiment changes are kept as patches. Before applying a patch, use `git apply --check` on its matching fixed revision. Copy-related permission and broken-symlink deltas are excluded from maintained dependencies; a clean checkout restores their upstream state. Third-party source trees, installers, and binaries are not vendored.

## OpenBLAS and generic CBLAS drivers

```bash
make -C benchmarks/kunpeng_blas/test-1023/Test_blas openblas \
  OPENBLAS_INC=/opt/OpenBLAS/include OPENBLAS_LIB=/opt/OpenBLAS/lib
make -C benchmarks/kunpeng_blas/test-1023/Test_blas blis \
  BLIS_INC=/usr/local/include/blis BLIS_LIB=/usr/local/lib
make -C benchmarks/kunpeng_blas/test-1023/Test_blas armpl \
  ARMPL_INC=/opt/arm/armpl_25.07_gcc/include ARMPL_LIB=/opt/arm/armpl_25.07_gcc/lib
```

The equivalent workspace under `BLAS_Openlib/Test_blas` is preserved. Shared drivers accept `--m`, `--n`, `--k`, `--repeats`, `--warmup`, `--layout row|col`, and `--transA N|T` / `--transB N|T`.

Makefiles also accept `OPENBLAS_LDLIBS`, `BLIS_LDLIBS`, and `ARMPL_LDLIBS`. These can point directly to a static archive. For the existing library tree on the second server:

```bash
dependency_root=/home/workspace/Yhaibiao/test-1023
make -C benchmarks/test_1023/Test_blas/openblas openblas \
  OPENBLAS_INC="$dependency_root/OpenBLAS" \
  OPENBLAS_LIB="$dependency_root/OpenBLAS" \
  OPENBLAS_LDLIBS="$dependency_root/OpenBLAS/libopenblas_armv8p-r0.3.30.dev.a -lm -lpthread"
```

Dynamic-library builds embed the configured library directory as an rpath. `libenv.sh` can also be sourced to set overridable library prefixes and thread defaults.

## Backend-specific drivers

### BLIS

```bash
make -C benchmarks/test_1023/Test_blas/blis BLIS_DIR=/usr/local
OMP_NUM_THREADS=2 benchmarks/test_1023/Test_blas/blis/blis_bench_dgemm 32 48 16 1 5
```

For an existing source-tree build, set `BLIS_INC`, `BLIS_LIB`, and `BLIS_LDLIBS` to `blis/include/armsve`, `blis/lib/armsve`, and the corresponding static archive.

The first workspace's `Test_blas/blis` directory builds two binaries: `dgemm_bench_blis` takes named arguments; `dgemm_nt_bench_blis` takes `M N K repeat warmup threads`. Their runners keep these interfaces separate.

### ARMPL

```bash
make -C benchmarks/test_1023/Test_blas/armpl ARMPL_DIR=/opt/arm/armpl_25.07_gcc
OMP_NUM_THREADS=2 benchmarks/test_1023/Test_blas/armpl/armpl_bench_dgemm 32 48 16 1 5
```

The Makefile selects ARMPL's threaded LP64 library when available. `OMP_LIB=gomp` matches the GCC runtime. The dedicated ARMPL and BLIS drivers write diagnostic output to stderr and measurement rows to stdout.

### LIBXSMM

```bash
make -C benchmarks/test_1023/Test_blas/libxsmm LIBXSMM_ROOT=/opt/libxsmm
OMP_NUM_THREADS=2 benchmarks/test_1023/Test_blas/libxsmm/gemm_bench_libxsmm_nt 17 19 13 2 1
```

The final argument enables a scalar reference calculation. A reference mismatch returns a nonzero exit code. `LIBXSMM_INC`, `LIBXSMM_LIB`, and `LIBXSMM_LDLIBS` support existing source-tree builds. The first workspace's `BLAS_Openlib/Test_blas/libxsmm/Makefile` reuses this maintained source.

### BLASFEO and LibShalom

```bash
make -C benchmarks/kunpeng_blas/test-1023/Test_blas/blasfeo BLASFEO_PREFIX=/usr/local
OMP_NUM_THREADS=2 benchmarks/kunpeng_blas/test-1023/Test_blas/blasfeo/bench_blasfeo_dgemm_nt 17 23 13 2 1 2 7

make -C benchmarks/kunpeng_blas/test-1023/Test_blas/shalom LibShalom_PREFIX=/opt/LibShalom
benchmarks/kunpeng_blas/test-1023/Test_blas/shalom/dgemm_bench_shalom \
  --m 32 --n 48 --k 16 --threads 2 --repeats 2 --warmup 1 --verify
```

BLASFEO uses an outer OpenMP loop and N-direction slicing. LibShalom's FP64 backend provides NN GEMM; the driver preserves its native tiled path and finishes uncovered edge elements with a scalar cleanup. The native DGEMM implementation is serial; requested LibShalom thread settings must not be interpreted as proof of FP64 parallel scaling. LibShalom output filenames use `NN` to identify this operation.

## NEON and SVE probes

```bash
mkdir -p build
gcc -O3 -march=native -ffast-math -fopenmp benchmarks/test_1023/fp64_peak.c -o build/fp64_peak
gcc -O3 -march=native -ffast-math -fopenmp benchmarks/test_1023/fp64_peak_sve.c -o build/fp64_peak_sve
OMP_NUM_THREADS=2 OMP_DYNAMIC=false build/fp64_peak 100000
OMP_NUM_THREADS=2 OMP_DYNAMIC=false build/fp64_peak_sve 100000
```

Both probes execute 16 independent FMA instructions per iteration and count the actual OpenMP team size. NEON counts `16 × 2 lanes × 2 FLOPs`; SVE uses the detected FP64 lane count. Thread-local SVE values keep the OpenMP region compatible with the servers' GCC 10.3.1.

A hardware peak measurement additionally requires an appropriate run duration, stable clock frequency, CPU affinity, and control of competing workloads. The short regression runs check execution and accounting.

## Full experiment runs

Use the runner's stored shape lists, or set `SHAPES_FILE` for the generic CBLAS runners. Set library and OpenMP thread counts consistently. Apply NUMA and CPU binding to match the experiment being reproduced; `NUMA_PREFIX` is optional for the dedicated BLIS / ARMPL runners.

New output is timestamped by default. `OUT` selects an explicit destination. Compare experiments with matching layouts, shapes, thread placement, and timing statistics. Original measurements and workbooks remain unchanged in [the dataset collection](../data/README.md).
