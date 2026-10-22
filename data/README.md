# Experiment datasets

Original library measurements and comparison workbooks for the GEMM experiments. Files retain their source values and experiment parameters; maintenance runs write to separate output files.

## Source collections

| Collection | CSV files | Measurement rows | Scope |
| --- | ---: | ---: | --- |
| [experiment_exports/kp920_5250](experiment_exports/kp920_5250/) | 3 | 47 | Additional KP920-5250 OpenBLAS exports |
| [kunpeng_blas/BLAS_Openlib](kunpeng_blas/BLAS_Openlib/) | 16 | 310 | Original BLAS_Openlib workspace records |
| [kunpeng_blas/test-1023](kunpeng_blas/test-1023/) | 11 | 161 | Original test-1023 workspace records |
| [library_runs/amd_epyc_7663](library_runs/amd_epyc_7663/) | 7 | 237 | AMD EPYC 7663: OpenBLAS, BLIS, LIBXSMM |
| [library_runs/kp920_5250](library_runs/kp920_5250/) | 18 | 336 | KP920-5250: OpenBLAS, BLIS, ARMPL, LIBXSMM |
| [library_runs/kp920_7285z](library_runs/kp920_7285z/) | 9 | 356 | KP920-7285Z: OpenBLAS, BLIS, ARMPL, LIBXSMM |
| [test_1023/Test_blas](test_1023/Test_blas/) | 12 | 376 | Second server workspace records |

Counts describe archived files, not independent trials. Some files are copies or partial captures of the same experiment; use source paths, timestamps, shapes, and thread counts to select a run before aggregating.

## Original workbooks

| Workbook | Contents |
| --- | --- |
| [Kunpeng 920-F results](workbooks/k1920_kp920_f.xlsx) | Original three-sheet comparison workbook |
| [Cross-platform comparison](workbooks/k1920_comparison.xlsx) | K=384 / K=1920 summaries, library comparisons, KML_BLAS and RippleGEMM entries |
| [AMD EPYC 7663 results](workbooks/k1920_amd_epyc_7663.xlsx) | Original AMD comparison sheet |

Workbook formulas, cell values, and sheet names are preserved byte for byte. A library named in a comparison workbook does not imply that a standalone benchmark driver for that library is included.

## Reading CSV files

Headers differ between the original runners. Generic CBLAS captures commonly contain `m,n,k,threads,gflops`; dedicated drivers may include `time_sec`, `precision`, or a repetition count. Historical diagnostic lines beginning with `#` should be skipped:

```python
import csv

with open("path/to/results.csv") as stream:
    rows = list(csv.DictReader(line for line in stream if line.strip() and not line.lstrip().startswith("#")))
```

Three source files contain only a header and contribute zero measurement rows. Keep timing conventions distinct: the original drivers use mean, minimum, or trimmed-mean time, while the paper describes its own evaluation protocol.

HPL and LibShalom text outputs are preserved under the server source labels. SHA-256 and source-folder mappings are recorded in `docs/provenance/` at the repository root. The plotted paper results are in [exp_result](../exp_result/).
