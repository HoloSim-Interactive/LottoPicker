---
name: data-out-302-backtest-report-structure
description: TP-DATA-OUT-302 (issue #22) verification — dedicated tagged test plus a real multi-date CLI run; note SE's stated 85/85 vs actual 84/84 test-case count.
metadata:
  type: project
---

Verified TP-DATA-OUT-302 (issue #22): fixture backtest across 3 synthetic
years, structure must have exactly one row per year, all fields
populated, consistent with TP-CORE-205's fixture results.

**What I checked:**
1. `[DATA-OUT-302][CORE-205]`-tagged case in `tests/BacktestEngineTest.cpp`
   (single Catch2 test case, 85 assertions) reproduces TP-CORE-205 part
   1's hand-verified 20/45/18/1 containment breakdown independently for
   each of 3 years — matched by re-deriving by hand, not just trusting
   the code.
2. Full suite: `ctest --preset default` → 84/84; raw binary → 721
   assertions in 84 test cases, `clang-format --dry-run --Werror` clean
   on all 3 touched files.
3. Real CLI end-to-end: built a throwaway 8-draw CSV + config, ran
   `lottopicker <config> --backtest d1,d2,d3` — confirmed exactly 3
   printed rows, one per requested date, each with rank/containment
   fields populated (all "not found" here since the synthetic draws
   don't overlap at top_n=10/pool 48, but the row *shape* is what
   DATA-OUT-302 gates, not specific values — those are TP-CORE-205's
   job). Multi-date backtest runs are slow (~50-60s per date on a
   pool-48+ full-space scan per [[core_203_ranking_engine]]/[[core_205_backtest_engine]]
   notes) — 3 dates exceeded the 120s default Bash timeout; ran it via
   `run_in_background` and read the output file once notified, rather
   than raising the timeout or chaining sleeps.

**Minor discrepancy, not a fail ground:** Software Engineer's handoff
comment said "85/85 test cases, 721 assertions" but the actual run is
84/84 test cases (matching 721 assertions exactly) — looks like an
off-by-one typo in the comment, not a real extra/missing test. Worth
independently recounting `ctest --preset default`'s tail line rather
than trusting a quoted count when it's this close to matching.

**Trunk regression pass, second hand-off (2026-09-05):** after CI/CD
merged `issue-22` to `main` at df2337c (tag v1.0.204) and SE recorded
the SHA in RTVM (b0765e1), routed back for regression. `main` @
b0765e1, clean rebuild from scratch, 84/84 ctest pass (unchanged
count — no concurrent work landed in between). Re-ran the
`[DATA-OUT-302][CORE-205]` case directly (85 assertions, pass) and
redid the real end-to-end `--backtest` CLI check with a fresh 8-draw
fixture — again exactly 3 rows, all fields populated. Config file
must be `key = value` lines (e.g. `data_file = draws.csv`), not JSON —
tried JSON first and got `missing required config key: data_file`.
Standard fast path holds.
