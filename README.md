# LottoPicker

A single-process, offline C++ CLI tool that ranks number combinations
for Florida Lotto (6-of-`n`) using historical draw data, and backtests
that ranking against past draws. See `docs/PROJECT_DEFINITION.md` for
the full scope and `docs/SDD.md` for the architecture.

> **Status:** scaffolding only (Generate Code Base). The sections below
> are filled in for real by `DELIV-900` once a working end-to-end
> example exists — see `docs/RTVM.md` for that item's status.

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

*TODO (DELIV-900): one complete worked example here once
`examples/sample_history.csv` exists, per `docs/SDD.md`'s `UI-002`
format (`data_file=...`, `top_n=...`).*

## Launch command

*TODO (DELIV-900): the exact invocation against the sample config,
covering both the ranking run and a `--backtest` run, per
`docs/SDD.md`'s `UI-003` CLI contract.*

## Sample output

*TODO (DELIV-900): a literal example of correct console output for
both run modes (`OUT-400`, `OUT-401`).*

## Project documents

- [`docs/PROJECT_DEFINITION.md`](docs/PROJECT_DEFINITION.md) — scope,
  stakeholder needs, MVP definition.
- [`docs/RTVM.md`](docs/RTVM.md) — requirements traceability &
  verification matrix.
- [`docs/SDD.md`](docs/SDD.md) — architecture, coding standards, build
  & toolchain conventions.
- [`docs/IMPLEMENTATION_PLAN.md`](docs/IMPLEMENTATION_PLAN.md) — build
  sequencing.
