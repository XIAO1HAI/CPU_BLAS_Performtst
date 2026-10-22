#!/usr/bin/env bash
set -euo pipefail

# 配置：选择库与线程
LIB=${1:-openblas}   # openblas / blis
THREADS=${2:-16}     # 根据物理核数调整
BIN="./dgemm_bench_${LIB}"

if [ ! -x "$BIN" ]; then
  echo "Binary $BIN not found. Build with: make $LIB"; exit 1
fi

# 线程与亲和性
export OMP_NUM_THREADS=$THREADS
export OMP_PROC_BIND=close
export OMP_PLACES=cores

if [ "$LIB" = "openblas" ]; then
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
  "384 83584 384"
  "384 83200 384"
  "384 81646 384"
  "384 79744 384"
  "384 77824 384"
  "384 75520 384"
  "384 73216 384"
  "384 66304 384"
  "384 63232 384"
  "384 60544 384"
  "384 59008 384"
  "384 56704 384"
  "384 50944 384"
  "384 47872 384"
  "384 44416 384"
  "384 43264 384"
  "384 41344 384"
  "384 39040 384"
  "384 34432 384"
  "384 32512 384"
  "384 29440 384"
  "384 27904 384"
  "384 24064 384"
  "384 22912 384"
    "2688 77440 1920"
    "2688 83200 1920"
    "2688 74368 1920"
    "2688 80128 1920"
    "2688 31360 1920"
    "2688 47104 1920"
    "2688 44416 1920"
    "2688 83968 1920"
    "2688 82048 1920"
    "2688 79744 1920"
    "2688 78976 1920"
    "2688 83584 1920"
    "2688 81664 1920"
    "2688 81280 1920"
    "2688 75520 1920"
    "3456 83584 1920"
    "3456 77440 1920"
    "3456 78208 1920"
    "3456 80512 1920"
    "3456 45568 1920"
    "3456 45184 1920"
    "3456 37888 1920"
    "12672 62464 1920"
    "12672 73984 1920"
    "12672 64000 1920"
    "10368 77824 1920"
    "10368 75904 1920"
    "10368 80896 1920"
    "8064 76288 1920"
    "8064 74752 1920"
    "8064 62424 1920"
    "13440 28288 1920"
    "13440 25216 1920"
    "13440 22144 1920"
    "13440 17152 1920"
    "13440 14080 1920"
    "13056 14080 1920"
    "13056 17152 1920"
    "13056 28288 1920"
    "13056 20224 1920"
    "13056 25216 1920"
    "12672 11008 1920"
    "12672 16000 1920"
    "12672 19072 1920"
    "12672 5632 1920"
    "12672 14080 1920"
    "12672 17152 1920"
)

###test 验证结果平稳是正常现象
# SHAPES=("51200 51200 51200") # 36,356.38
# SHAPES=("16 512 512") # 36,47.12

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


