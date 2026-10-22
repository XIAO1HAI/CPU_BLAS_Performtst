"""Local regression checks; no BLAS installation or benchmark hardware required."""
from pathlib import Path
import csv
import os
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class BenchmarkRegressionTests(unittest.TestCase):
    def test_shell_syntax(self):
        for script in (ROOT / 'benchmarks').rglob('*.sh'):
            with self.subTest(script=script):
                result = subprocess.run(['bash', '-n', str(script)], capture_output=True, text=True)
                self.assertEqual(result.returncode, 0, result.stderr)

    def test_makefile_targets_resolve(self):
        for makefile in (ROOT / 'benchmarks').rglob('Makefile'):
            with self.subTest(makefile=makefile):
                result = subprocess.run(['make', '-n', '-C', str(makefile.parent)], capture_output=True, text=True)
                self.assertEqual(result.returncode, 0, result.stderr)

    def test_environment_can_be_sourced_with_unset_variables(self):
        for script in (ROOT / 'benchmarks').rglob('libenv.sh'):
            command = 'set -eu; unset LD_LIBRARY_PATH; source "$1"; test "$OMP_NUM_THREADS" -gt 0'
            result = subprocess.run(['bash', '-c', command, 'check', str(script)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)

    def test_integer_and_matrix_size_validation(self):
        with tempfile.TemporaryDirectory() as temp:
            source = Path(temp) / 'args.c'
            binary = Path(temp) / 'args'
            source.write_text('#include "benchmark_args.h"\nint main(int argc, char **argv) {\n'
                              'if (argc != 3) return 2;\n'
                              'bench_elements(bench_int(argv[1], 1), bench_int(argv[2], 1));\nreturn 0;\n}\n')
            subprocess.run(['cc', '-I', str(ROOT / 'benchmarks'), str(source), '-o', str(binary)], check=True)
            self.assertEqual(subprocess.run([str(binary), '17', '19']).returncode, 0)
            for args in [('0', '19'), ('-1', '19'), ('abc', '19'), ('9999999999999', '19'), ('2147483647', '2147483647')]:
                result = subprocess.run([str(binary), *args], capture_output=True)
                self.assertEqual(result.returncode, 1, args)

    def test_cblas_driver_rejects_empty_or_invalid_transpose(self):
        with tempfile.TemporaryDirectory() as temp:
            header = Path(temp) / 'cblas.h'
            header.write_text('enum CBLAS_ORDER {CblasRowMajor=101,CblasColMajor=102};\n'
                              'enum CBLAS_TRANSPOSE {CblasNoTrans=111,CblasTrans=112};\n'
                              'static inline void cblas_dgemm(int l,int ta,int tb,int m,int n,int k,\n'
                              'double a,const double*A,int lda,const double*B,int ldb,double b,double*C,int ldc) {}\n')
            binary = Path(temp) / 'driver'
            source = ROOT / 'benchmarks/test_1023/Test_blas/openblas/dgemm_bench.c'
            subprocess.run(['cc', '-I', temp, str(source), '-o', str(binary)], check=True)
            for value in ['', 'Q', 'NN']:
                result = subprocess.run([str(binary), '--transB', value], capture_output=True)
                self.assertEqual(result.returncode, 1, value)

    def test_positional_blis_runner_writes_numeric_csv(self):
        script = ROOT / 'benchmarks/kunpeng_blas/test-1023/Test_blas/blis/run_nt_blis.sh'
        with tempfile.TemporaryDirectory() as temp:
            directory = Path(temp)
            shutil.copyfile(script, directory / script.name)
            (directory / 'shapes.csv').write_text('# M,N,K\n17,19,13\n')
            fake = directory / 'dgemm_nt_bench_blis'
            fake.write_text('#!/usr/bin/env bash\n[[ $# == 6 ]] || exit 7\n'
                            'echo "BLIS DGEMM NT M=$1 N=$2 K=$3 thr=$6 time=0.001 s GFLOPS=1.25"\n')
            fake.chmod(0o755)
            output = directory / 'result.csv'
            env = dict(os.environ, OUT=str(output), THREADS_LIST='2', ITERS='2', WARMUP='0')
            result = subprocess.run(['bash', script.name], cwd=directory, env=env, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            with output.open() as stream:
                row = next(csv.DictReader(stream))
            self.assertEqual(float(row['GFLOPS']), 1.25)
            self.assertEqual(float(row['time']), 0.001)

    def test_blis_runner_executes_without_a_numa_prefix(self):
        script = ROOT / 'benchmarks/test_1023/Test_blas/blis/run_blis_bench.sh'
        with tempfile.TemporaryDirectory() as temp:
            directory = Path(temp)
            fake = directory / 'blis_bench_dgemm'
            fake.write_text('#!/usr/bin/env bash\n[[ $# == 5 ]] || exit 7\n'
                            'echo "# diagnostic stays on stderr" >&2\n'
                            'echo "$1,$2,$3,${OMP_NUM_THREADS},dgemm,0.001,1.25"\n')
            fake.chmod(0o755)
            output = directory / 'result.csv'
            result = subprocess.run(['bash', str(script)], cwd=directory,
                                    env=dict(os.environ, OUT=str(output), NUMA_PREFIX=''), capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            with output.open() as stream:
                rows = list(csv.DictReader(stream))
            self.assertGreater(len(rows), 1)
            self.assertTrue(all(float(row['GFLOPS']) == 1.25 for row in rows))


if __name__ == '__main__':
    unittest.main()
