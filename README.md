# CPU_BLAS_Performtst

Kunpeng CPU 上的 BLAS / DGEMM 性能测试源码、原始实验记录和依赖版本快照。整理自 `KunPeng_BLAS` 与另一台服务器的 `test-1023` 两个工作目录，保留开发时的不同版本与矩阵形状。

主要测试 OpenBLAS、BLIS、Arm Performance Libraries、LIBXSMM，另有 BLASFEO、LibShalom 测试入口、NEON / SVE 合成测试和 HPL 配置。这里归档的是指定目录内找到的代码；尚未确认包含论文 RippleGEMM 的完整实现或 `ripplegemm_dgemm` API。

## 目录

| 路径 | 内容 |
| --- | --- |
| `benchmarks/kunpeng_blas/` | 第一处工作目录的测试源码；保留 `BLAS_Openlib` 与 `test-1023` 两套版本 |
| `benchmarks/test_1023/` | 第二处工作目录的 OpenBLAS、BLIS、ARMPL、LIBXSMM 与峰值测试 |
| `data/` | 原始结果 CSV、LibShalom 文本结果和 HPL 输出，按来源保留相对路径 |
| `exp_result/` | 本地图示目录中的 7 张相关实验图，原样复制 |
| `dependencies/` | 第三方库的仓库地址、固定提交、工作区补丁、构建配置和许可证 |
| `docs/environments/` | 导入时的 CPU、操作系统及编译器信息 |
| `docs/provenance/` | 来源路径与逐文件 SHA-256，用于核对导入内容 |

第三方库本体、安装包、目标文件、可执行文件、动态库、编辑器配置和临时文件不入库。HPL 仅保留相对原始 2.3 发布包新增的用户配置与结果。原始源码和实验数值不作修改，重复版本也不合并。

## 快速开始

需要 Linux / AArch64、GCC、Bash、Make，以及提供 CBLAS 接口的 BLAS 库。默认使用服务器原有安装前缀，路径不同时可在 `make` 命令行覆盖。

```bash
cd benchmarks/kunpeng_blas/test-1023/Test_blas
make openblas OPENBLAS_INC=/opt/OpenBLAS/include OPENBLAS_LIB=/opt/OpenBLAS/lib
export LD_LIBRARY_PATH="/opt/OpenBLAS/lib:${LD_LIBRARY_PATH:-}"
export OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1 GOTO_NUM_THREADS=1
./dgemm_bench_openblas --m 32 --n 48 --k 16 \
  --layout row --transA N --transB T --repeats 2 --warmup 1
```

小矩阵仅用于检查编译、链接和运行。完整形状测试使用对应目录内的运行脚本，例如 `bash run_nt.sh openblas 36`；脚本通常会覆盖当前目录的同名结果文件。历史结果独立保存在 `data/` 中。

第二处工作目录的库保存在源码树内，没有安装到默认 `/opt` 路径，参见[运行与依赖说明](docs/reproduction.md)。

## 实验说明与验证

常见形状包含固定 `K=384`、`K=1920` 的 NT DGEMM。不同程序分别使用平均时间、去掉极值后的平均时间或最短时间，重复次数也不一致；不能直接视为论文所述的统一统计流程。

已核对源码及数据的 SHA-256，检查原始 CSV、PNG 格式，并在两台服务器上执行部分编译和单线程小矩阵检查。完整结果及原有问题见[验证记录](docs/validation.md)和[已知问题](docs/known_issues.md)。本次整理没有重新测量整套性能数据，也没有确认每张图与每个原始 CSV 的一一对应关系。

关联论文：HaiBiao Yang et al., *RippleGEMM: Normalizing irregular DGEMM on CPUs via execution-layer optimization mechanisms*, Journal of Systems Architecture 178 (2026), 103902. [DOI](https://doi.org/10.1016/j.sysarc.2026.103902)。论文用于核对背景与实验说明，PDF 未纳入本次代码归档。

## 许可证

保留仓库原有 [MIT License](LICENSE)。第三方代码、补丁涉及的原始代码和依赖保留其各自许可证，见 `dependencies/*/licenses/`；论文和实验图不因本次整理而改变原有权利归属。
