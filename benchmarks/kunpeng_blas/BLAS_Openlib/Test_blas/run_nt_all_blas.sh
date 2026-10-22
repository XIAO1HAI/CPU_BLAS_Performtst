#!/usr/bin/env bash
# run_nt.sh
# 用法：
#   bash run_nt.sh <lib> <threads>
#   <lib> = openblas | blis | armpl | shalom
#   <threads> = 线程数（建议为物理核数；Kunpeng 920 单 NUMA 节点按需设置）

set -euo pipefail

LIB=${1:-openblas}
THREADS=${2:-64}

# 找到对应的可执行文件名
case "$LIB" in
  openblas) BIN=./dgemm_bench_openblas; MAKE_TGT=openblas ;;
  blis)     BIN=./dgemm_bench_blis;     MAKE_TGT=blis     ;;
  armpl)    BIN=./dgemm_bench_armpl;    MAKE_TGT=armpl    ;;
  shalom)   BIN=./dgemm_bench_shalom;   MAKE_TGT=shalom   ;;
  *) echo "Unknown lib: $LIB (openblas|blis|armpl|shalom)"; exit 1;;
esac

# 若二进制不存在则编译
if [ ! -x "$BIN" ]; then
  echo "[INFO] $BIN not found. Building with: make $MAKE_TGT"
  make "$MAKE_TGT"
fi

# 线程与亲和性（统一设置）
export OMP_NUM_THREADS=$THREADS
export OMP_PROC_BIND=close
export OMP_PLACES=cores

if [ "$LIB" = "openblas" ]; then
  export OPENBLAS_NUM_THREADS=$THREADS
  export GOTO_NUM_THREADS=$THREADS
elif [ "$LIB" = "blis" ]; then
  export BLIS_NUM_THREADS=$THREADS
elif [ "$LIB" = "armpl" ]; then
  # ARMPL 也是 OpenMP，OMP_NUM_THREADS 即可
  true
elif [ "$LIB" = "shalom" ]; then
  # 对 shalom 我们把线程数通过命令行 --threads 传给二进制（也可改为读环境）
  true
fi

# 形状列表（按需增删）
SHAPES=(
  "384 83968 384"   # large-k
)

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

# 尝试用 numactl 绑到 NUMA 节点 0，如不存在则直接运行
RUNNER=()
if command -v numactl >/dev/null 2>&1; then
  RUNNER=(numactl --cpunodebind=0 --membind=0)
fi

for s in "${SHAPES[@]}"; do
  read -r m n k <<< "$s"

  if [ "$LIB" = "shalom" ]; then
    # LibShalom 版本接受 --threads
    RES=$("${RUNNER[@]}" "$BIN" --m "$m" --n "$n" --k "$k" --repeats 5 --warmup 1 --threads "$THREADS")
  else
    RES=$("${RUNNER[@]}" "$BIN" --m "$m" --n "$n" --k "$k" --layout row --transA N --transB T --repeats 5 --warmup 1)
  fi

  echo "$RES"
  # 提取 GFLOP/s 数值（行末 "GFLOP/s=xx.xx"）
  GF=$(echo "$RES" | awk '{for(i=1;i<=NF;i++){ if ($i ~ /GFLOP\/s=/){ split($i,a,"="); print a[2] }}}')
  echo "$m,$n,$k,$THREADS,$GF" >> "$OUT"
done

echo "[DONE] Saved: $OUT"
