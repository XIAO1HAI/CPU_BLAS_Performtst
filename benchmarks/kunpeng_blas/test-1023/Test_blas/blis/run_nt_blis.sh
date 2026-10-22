#!/usr/bin/env bash
set -euo pipefail

BIN=./dgemm_nt_bench_blis
SHAPES=./shapes.csv
ITERS=${ITERS:-5}
WARMUP=${WARMUP:-2}
# 线程与绑核（按你的机器调整）
THREADS_LIST=${THREADS_LIST:-"36"}
# 绑核（可选）：ARM64 2 NUMA节点常见，示例固定在前 32/64 核
USE_TASKSET=0
BASE_CPUS="0-63"     # 请按机器真实核数修改
LOG="${OUT:-blis_nt_results_$(date +%Y%m%d_%H%M%S).csv}"

echo "M,N,K,threads,time,GFLOPS" | tee $LOG

while IFS=',' read -r M N K; do
  # 跳过注释或空行
  [[ -z "$M" ]] && continue
  [[ "$M" =~ ^# ]] && continue

  for T in $THREADS_LIST; do
    export BLIS_NUM_THREADS=$T
    if [[ $USE_TASKSET -eq 1 ]]; then
      OUT=$(taskset -c $BASE_CPUS $BIN $M $N $K $ITERS $WARMUP $T)
    else
      OUT=$($BIN $M $N $K $ITERS $WARMUP $T)
    fi
    echo "$OUT"
    # 解析输出行
    # 形如：BLIS DGEMM NT  M=... N=... K=...  thr=...  time=... s  GFLOPS=...
    TIME=$(echo "$OUT" | awk '{for(i=1;i<=NF;i++) if($i ~ /time=/){sub("time=","",$i); print $i}}')
    GF=$(echo "$OUT" | awk '{for(i=1;i<=NF;i++) if($i ~ /^GFLOPS=/){split($i,a,"="); print a[2]}}')
    echo "$M,$N,$K,$T,$TIME,$GF" | tee -a $LOG
  done
done < <(grep -v '^\s*$' "$SHAPES" | grep -v '^#')

