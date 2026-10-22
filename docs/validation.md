# Validation

The maintained drivers are checked independently from the historical performance datasets. Builds run in isolated temporary directories on the original servers; the original workspaces and measurement files remain unchanged.

## Local regression suite

```bash
python3 -m unittest discover -s tests -v
```

Seven checks cover all Shell scripts, Makefile target resolution, sourcing environment settings with unset variables, integer / allocation-range validation, the positional BLIS runner, and clean numeric CSV output without a NUMA command prefix.

## Hardware checks

GCC 10.3.1 is used on both Kunpeng servers. C drivers are compiled with implicit function declarations treated as errors. The matrix tests use two requested threads and small shapes so that execution checks remain separate from throughput experiments.

| Target | Evidence |
| --- | --- |
| Generic OpenBLAS / BLIS drivers | Both first-workspace variants compile and run; missing values, negative dimensions, malformed numbers, overflow, zero repeats, and invalid transpose modes are rejected |
| ARMPL | Generic drivers compile against the actual ARMPL headers; the dedicated driver also builds and runs using the installed threaded LP64 library |
| BLIS positional driver | Correct CBLAS entry point; non-square 17×19×13 shape runs successfully |
| LIBXSMM | 17×19×13, two repetitions, scalar-reference maximum absolute error below `1e-10` |
| BLASFEO | Non-square 17×23×13 with N-slices of 7, checked against independently generated scalar checksum values |
| LibShalom | Reference checks for 32×48×16, 17×23×13, 3×5×1, and the original 384×384×384 shape; maximum absolute errors at most `1.887e-15` in these runs |
| NEON | GCC compile/run on both servers, actual two-thread team reported, 16 independent `fmla` instructions present in generated assembly |
| SVE | GCC 10.3.1 OpenMP build and two-thread execution on the SVE-capable server; 16 `fmla` instructions in generated assembly |
| Makefiles | Seven actual build invocations cover the primary first-workspace drivers, both BLIS interfaces, shared LIBXSMM source, BLASFEO, and source-tree static libraries on the second server |
| OpenBLAS source patch | Applies to the matching fixed revision; its C and AArch64 assembly files compile successfully |

Machine-readable records contain 29 hardware / build checks and the separate dependency-patch checks:

- [First workspace](checks/hardware_kunpeng_blas.json)
- [Second workspace](checks/hardware_test_1023.json)
- [Dependency patch](checks/dependency_patch_validation.json)

The LibShalom FP64 native kernel is serial; reference correctness checks are independent of parallel-scaling claims. Synthetic throughput checks validate instruction accounting and execution, with clock / affinity calibration specified in the reproduction guide.

## Data and figures

- 76 original result CSV files, totaling 1,823 non-comment measurement rows, preserve their source values.
- Three original comparison workbooks pass ZIP-container integrity checks and retain their original formulas and worksheet names.
- Four method figures are exported from the supplied published PDF with a white background; their PDF page, figure number, and source-PDF checksum are recorded.
- Seven experimental-result PNGs retain their original bytes. The 24-ID figure represents KP920-F with M=K=384, as identified by the paper's Figure 10 and Table 3.
- Import-time checksums remain in `docs/provenance/`; maintained source revisions have additional `maintained_sha256` fields. Removed transfer-only patches are explicitly marked in the import records.

[Dataset index](../data/README.md) · [Build and run](reproduction.md)

Full performance sweeps, frequency stabilization, and paper-wide result reproduction are outside these maintenance checks. Historical results are not replaced with small-matrix check output.
