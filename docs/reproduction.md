# 运行与依赖说明

## 两处来源

| 归档标签 | 原始目录 | 导入时观测到的硬件 |
| --- | --- | --- |
| `kunpeng_blas` | `/home/workspace/Yhaibiao/KunPeng_BLAS` | BIOS 报告 Kunpeng 920 7265，256 个逻辑 CPU，4 sockets，8 NUMA nodes |
| `test_1023` | `/home/workspace/Yhaibiao/test-1023` | BIOS 报告 Kunpeng 920 7285Z，640 个逻辑 CPU，320 个物理核，4 sockets，8 NUMA nodes |

两台服务器检测到的 GCC 均为 10.3.1。以上是导入时的环境，不用它倒推历史 CSV 的硬件型号；第一处目录中的结果尤其不能仅凭当前位置标为 KP920-5250 或 KP920-F。完整输出在 `environments/`。

## 固定依赖

每个来源的 `dependencies/<标签>/sources.json` 记录第三方库相对路径、仓库地址和完整提交号。同名库可能存在多个工作副本，均分别记录。

| 依赖 | 测试目录中的常见提交 |
| --- | --- |
| OpenBLAS | `aef36a3ff00842b09c0ef706c97d8fda10ef9584` |
| BLIS | `daa6e41ff2a6798bb4b5fa646a35f748bb8b0eae` |
| LIBXSMM | `10b7dc82b3c46157e76eb40e4e959555f895b24d` |
| LibShalom | `9b16391d68370cca3dde53dcb687576336ecfc64` |
| BLASFEO | `4e0d2f317174aae8fdc15059fe404fc9155bd5f3` |

例如，准备与测试目录对应的 OpenBLAS 源码：

```bash
mkdir -p third_party
git clone https://gitee.com/xiao--hai/OpenBLAS.git third_party/OpenBLAS
git -C third_party/OpenBLAS checkout --detach aef36a3ff00842b09c0ef706c97d8fda10ef9584
```

库的构建和安装按照该固定提交的上游说明执行。`build_config/` 保存原工作副本的部分生成配置，便于检查编译选项；这些文件并不证明已安装库的二进制恰由该工作副本生成。

`patches/` 为 `git diff --binary HEAD` 的原始输出。它们用于保留工作区状态，默认不作为性能测试必须应用的优化：第一处顶层 OpenBLAS 的补丁带有原有语法问题，第二处的补丁还包含文件权限及符号链接损坏，详见已知问题。如需重建某个工作区，在对应固定提交上先运行 `git apply --check <补丁绝对路径>`，再决定是否应用。LibShalom 新增文件位于 `benchmarks/` 对应相对路径，可覆盖到匹配的依赖工作副本。

HPL 使用 Netlib HPL 2.3 发布包。`sources.json` 保存服务器上原始 `hpl-2.3.tar.gz` 的 SHA-256；归档的 `Make.Linux_PII_CBLAS` 与 `HPL.dat` 可覆盖到解压后的对应位置。MPI、BLAS 的安装前缀需按实际环境修改。生成的 Makefile、构建标记、MPI 安装目录和第三方压缩包未纳入归档。

服务器中的 ARMPL 安装包为 25.07 GCC 版，未重新分发。请从 Arm 官方渠道获取，并遵循其许可证。

## 第一处目录

`benchmarks/kunpeng_blas/test-1023/Test_blas` 与 `benchmarks/kunpeng_blas/BLAS_Openlib/Test_blas` 保留原有版本差异。均可通过 `make openblas` 或 `make blis` 编译主测试，安装路径用 `OPENBLAS_INC`、`OPENBLAS_LIB`、`BLIS_INC`、`BLIS_LIB` 覆盖。

运行脚本应在各自 Makefile 所在目录调用。主程序支持 `--m`、`--n`、`--k`、`--layout`、`--transA`、`--transB`、`--repeats`、`--warmup`。请使用完整、正数参数；原始解析器未全面校验输入。

BLIS 专用目录的 `dgemm_nt_bench_blis` 使用位置参数：

```bash
make -C benchmarks/kunpeng_blas/test-1023/Test_blas/blis BLIS_HOME=/usr/local
cd benchmarks/kunpeng_blas/test-1023/Test_blas/blis
./dgemm_nt_bench_blis 32 48 16 2 1 1
```

同目录 `shapes.csv` 是运行输入，保留在源码旁；结果 CSV 独立存入 `data/`。不应将参数格式不同的主测试脚本用于这一专用二进制。

## 第二处目录

导入时 `/opt/OpenBLAS`、`/opt/libxsmm`、`/usr/local/include/blis` 和脚本默认 ARMPL 前缀不可用。服务器工作目录中保留了相应依赖的源码树和部分已编译库，因此在该服务器验证时使用实际路径。

现有 OpenBLAS / LIBXSMM 的部分库名软链接变成了短文本文件。验证时直接链接有效的静态库，避免依赖这些损坏的链接。例如在第二台服务器、归档仓库根目录执行：

```bash
dependency_root=/home/workspace/Yhaibiao/test-1023
mkdir -p build
gcc -O3 -march=native -ffast-math -fopenmp \
  -I"$dependency_root/OpenBLAS" \
  benchmarks/test_1023/Test_blas/openblas/dgemm_bench.c \
  "$dependency_root/OpenBLAS/libopenblas_armv8p-r0.3.30.dev.a" \
  -lm -lpthread -o build/dgemm_bench_openblas
OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1 GOTO_NUM_THREADS=1 \
  build/dgemm_bench_openblas --m 32 --n 48 --k 16 \
  --layout row --transA N --transB T --repeats 2 --warmup 1
```

另外两个已验证的源码树内依赖路径相对 `dependency_root` 为：

| 库 | 头文件目录 | 静态库 | 额外链接选项 |
| --- | --- | --- | --- |
| BLIS | `blis/include/armsve` | `blis/lib/armsve/libblis.a` | `-lm -lpthread -fopenmp` |
| LIBXSMM | `libxsmm/include` | `libxsmm/lib/libxsmm.a` | `-lm -ldl -lpthread -fopenmp` |

ARMPL 目录只找到安装包，没有可直接链接的已安装库，本次未运行 ARMPL 基准。

对于重新安装到标准前缀的库，可以直接用原有 Makefile：

```bash
make -C benchmarks/test_1023/Test_blas/openblas openblas \
  OPENBLAS_INC=/opt/OpenBLAS/include OPENBLAS_LIB=/opt/OpenBLAS/lib
make -C benchmarks/test_1023/Test_blas/blis BLIS_DIR=/usr/local
make -C benchmarks/test_1023/Test_blas/armpl ARMPL_DIR=/opt/arm/armpl_25.07_gcc
make -C benchmarks/test_1023/Test_blas/libxsmm
```

LIBXSMM Makefile 的安装路径写在 `CFLAGS`、`LDFLAGS` 内；非默认路径需覆盖这两个变量。对应程序启用参考结果检查的示例：

```bash
export OMP_NUM_THREADS=1
./gemm_bench_libxsmm_nt 17 19 13 2 1
```

最后一个 `1` 启用朴素参考计算，程序打印最大绝对误差，但原程序不会按误差自动返回失败状态。

ARMPL、BLIS 专用程序均使用 `M N K [warmup] [repeat]` 位置参数；线程由 `OMP_NUM_THREADS` 等环境变量控制。运行完整脚本前应确认动态库路径、线程数、物理核布局与 NUMA 绑定，SVE 程序还要求处理器与工具链支持相应指令集。

## 原始结果与图

`data/` 中的 39 个结果 CSV 原样保留，不填补空结果、不清洗测量值。ARMPL / BLIS 的部分 CSV 混有以 `#` 开头的环境输出，解析时应跳过注释行。部分文件只有表头，不能计为测量结果。

`exp_result/` 原样复制自本地 `图示/exp_result`：

| 文件 | 图的主题 |
| --- | --- |
| `k1920_block{1,2,3}_3platform.png` | 三个平台的 K=1920 分组比较 |
| `bar_chart_24ids_3platform.png` | 24 个形状的三平台比较 |
| `peak_compare_all.png` | 峰值相关比较 |
| `micro-kernel-heatmap{1,2}.png` | 微内核效率热图 |

这些图不是本次运行生成的。尚未建立图中每个数据点与归档 CSV 的完整映射，不用烟雾测试结果替代论文测量。论文报告的 20 次测量取中位数，也不同于部分原程序的平均值、截尾平均值或最小值统计。
