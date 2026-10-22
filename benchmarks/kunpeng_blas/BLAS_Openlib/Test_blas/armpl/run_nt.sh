#!/usr/bin/env bash
set -euo pipefail

# 配置：选择库与线程
LIB=${1:-armpl}   # openblas / blis
THREADS=${2:-16}     # 根据物理核数调整
BIN="./dgemm_bench_${LIB}"

if [ ! -x "$BIN" ]; then
  echo "Binary $BIN not found. Build with: make $LIB"; exit 1
fi

# 线程与亲和性
export OMP_NUM_THREADS=$THREADS
export OMP_PROC_BIND=close
export OMP_PLACES=cores

if [ "$LIB" = "armpl" ]; then
  export OPENBLAS_NUM_THREADS=$THREADS
  export GOTO_NUM_THREADS=$THREADS
elif [ "$LIB" = "blis" ]; then
  export BLIS_NUM_THREADS=$THREADS
elif [ "$LIB" = "shalom" ]; then
  export OMP_NUM_THREADS=$THREADS
  # LibShalom 自身也有线程设置API；可选加个 env 让二进制读取并调用 LibShalom_set_thread_nums
fi

# 形状列表（可按需增删）
SHAPES=(
  # "384 512 512"
  # "1024 1024 1024"
  # "2048 2048 2048"
  # "4096 4096 4096"
  # "8192 8192 8192"
  # "8192 256 8192"    # tall-skinny
  # "256 8192 8192"    # short-wide
  # "4096 4096 256"    # small-k
  # "4096 4096 16384"  # large-k


)

# 输出CSV
if [[ -n "${SHAPES_FILE:-}" ]]; then
  mapfile -t SHAPES < <(awk -F, 'NF == 3 && $1 !~ /^[[:space:]]*#/ {print $1, $2, $3}' "$SHAPES_FILE")
fi
if (( ${#SHAPES[@]} == 0 )); then
  echo "No shapes configured; set SHAPES_FILE to an M,N,K CSV." >&2
  exit 1
fi

TRANSPOSE_TAG=NT
[[ "$LIB" != "shalom" ]] || TRANSPOSE_TAG=NN
OUT="${OUT:-results_${LIB}_${TRANSPOSE_TAG}_$(date +%Y%m%d_%H%M%S).csv}"
echo "m,n,k,threads,gflops" > "$OUT"

for s in "${SHAPES[@]}"; do
  read -r m n k <<< "$s"
  if [ "$LIB" = "shalom" ]; then
    # LibShalom 版本接受 --threads
    RES=$("$BIN" --m "$m" --n "$n" --k "$k" --repeats 5 --warmup 1 --threads "$THREADS")
  else
    RES=$("$BIN" --m "$m" --n "$n" --k "$k" --layout row --transA N --transB T --repeats 5 --warmup 1)
  fi
  # 绑到NUMA节点0（按需修改/移除）
#   RES=$(numactl --cpunodebind=0 --membind=0 \
#         "$BIN" --m "$m" --n "$n" --k "$k" --layout row --transA N --transB T --repeats 5 --warmup 1)
#   RES=$("$BIN" --m "$m" --n "$n" --k "$k" --layout row --transA N --transB T --repeats 5 --warmup 1)
  echo "$RES"
  GF=$(echo "$RES" | awk '{for(i=1;i<=NF;i++) if ($i ~ /GFLOP\/s=/) {split($i,a,"="); print a[2]}}')
  echo "$m,$n,$k,$THREADS,$GF" >> "$OUT"
done

echo "Saved: $OUT"

