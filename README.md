# LottoPicker

A single-process, offline C++ CLI tool that ranks number combinations
for Florida Lotto (6-of-`n`) using historical draw data, and backtests
that ranking against past draws. See `docs/PROJECT_DEFINITION.md` for
the full scope and `docs/SDD.md` for the architecture.

## Build from a clean clone

Requires CMake 3.21+ and a C++17 compiler (GCC, Clang, or MSVC 2022).

```sh
cmake --preset default
cmake --build --preset default
ctest --preset default
```

On Windows, open the folder directly in Visual Studio 2022 (File → Open
→ Folder — VS's built-in CMake integration auto-detects
`CMakeLists.txt` and the `windows-vs2022` preset), or run:

```sh
cmake --preset windows-vs2022
cmake --build --preset windows-vs2022
```

## Config file format

Flat `key=value` text, one entry per line; `#` starts a comment; blank
lines are ignored (`UI-002`). Two keys are required:

- `data_file` — path to the historical draw CSV, resolved relative to
  the *config file's own directory* (not wherever you run `lottopicker`
  from), so a config stays portable alongside its data.
- `top_n` — a positive integer: how many top-ranked combinations to
  keep and print.

Any other key is ignored (forward-compatible), not an error. A missing
`data_file`/`top_n`, or a `top_n` that isn't a positive integer, is a
validation error reported on stderr, naming the offending key.

This repo ships one complete working example under
[`examples/`](examples/) — see [`examples/README.md`](examples/README.md)
for what it contains:

`examples/sample_config.cfg`:
```
data_file=sample_history.csv
top_n=10
```

`examples/sample_history.csv` (first few lines; 15 rows total, header
`date,n1,n2,n3,n4,n5,n6`):
```
date,n1,n2,n3,n4,n5,n6
2024-01-03,4,11,17,22,33,41
2024-01-10,2,11,17,29,33,47
...
```

## Launch command

Build first (see above), then run the built `lottopicker` binary
against the sample config, from the repo root:

```sh
./build/default/src/lottopicker examples/sample_config.cfg
```

That's the normal ranking run (`OUT-400`). To also see the backtest
report (`OUT-401`), add `--backtest` with one or more
ISO-8601 (`YYYY-MM-DD`) sample dates, comma-separated for more than
one — each date must be one that actually appears in `data_file`, with
at least one earlier draw to train against:

```sh
./build/default/src/lottopicker examples/sample_config.cfg --backtest 2024-03-13
```

**Heads up on timing:** both commands above score the *full* ~22.9M-
combination search space (`C(53,6)`, `CORE-203`) every time they run,
regardless of how small the sample history is — this is expected, not
a hang, and normally takes **about a minute** per invocation (and once
per `--backtest` sample date) on a typical development machine. `model:
reused (source unchanged)` on a second run means the persisted model
artifact wasn't recomputed from raw history, but ranking still rescans
the full space to print the top-N.

## Sample output

Running the plain ranking command above against the shipped example
produces exactly this (all `top_n=10` rows shown):

```
lottopicker 0.1.0
config: examples/sample_config.cfg
data_file: examples/sample_history.csv
top_n: 10
model: rebuilt (15 draw(s)) -> examples/sample_history.csv.model
mode: rank
Rank | Combination       |      Score
1    | 4 11 17 22 33 41  | 488.637176
2    | 2 11 17 22 33 41  | 381.480731
3    | 2 11 17 29 33 47  | 305.709333
4    | 2 11 17 29 33 46  | 298.927965
5    | 4 11 17 29 33 48  | 284.611991
6    | 11 17 22 29 33 41 | 279.781869
7    | 11 14 17 22 33 41 | 277.971067
8    | 4 11 17 29 33 41  | 275.162487
9    | 5 11 17 22 33 41  | 274.101394
10   | 2 4 11 17 33 41   | 267.857039
```

Running the `--backtest 2024-03-13` command above produces:

```
lottopicker 0.1.0
config: examples/sample_config.cfg
data_file: examples/sample_history.csv
top_n: 10
model: reused (source unchanged) <- examples/sample_history.csv.model
mode: backtest (1 sample date(s))
Sample Date | Actual Draw      | Rank               | Containment (3/4/5/6 of 6)
2024-03-13  | 5 14 17 22 33 41 | not found in top-N | 4 1 0 0
```

(The `model:` line reads `rebuilt` instead of `reused` the very first
time you run either command against a fresh clone, since no
`examples/sample_history.csv.model` exists yet — see the "Heads up on
timing" note above.)

`not found in top-N` here is expected, not a bug: the actual
2024-03-13 draw simply didn't land in the truncated model's top 10 for
this small illustrative dataset (`top_n=10` in `sample_config.cfg`) —
`OUT-401` renders that case as literal text rather than a fabricated
rank. The `4 1 0 0` containment counts mean, among the top-10 ranked
combinations, 4 shared exactly 3 of the actual draw's 6 numbers and 1
shared exactly 4, with none sharing 5 or all 6.

Both outputs above are real captures from running the built binary
against the committed `examples/` files, not hand-typed — running the
same commands from a clean clone should reproduce them exactly (the
scoring pipeline is deterministic for unchanged input data and
weights).

## Project documents

- [`docs/PROJECT_DEFINITION.md`](docs/PROJECT_DEFINITION.md) — scope,
  stakeholder needs, MVP definition.
- [`docs/RTVM.md`](docs/RTVM.md) — requirements traceability &
  verification matrix.
- [`docs/SDD.md`](docs/SDD.md) — architecture, coding standards, build
  & toolchain conventions.
- [`docs/IMPLEMENTATION_PLAN.md`](docs/IMPLEMENTATION_PLAN.md) — build
  sequencing.
