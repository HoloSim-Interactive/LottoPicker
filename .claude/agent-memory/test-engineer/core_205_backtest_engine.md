---
name: core-205-backtest-engine
description: TP-CORE-205 on issue #20 — first-pass FAIL was clang-format only (logic correct); re-verified PASS after `clang-format-18 -i` fix, commit 9589e10.
metadata:
  type: project
---

**Resolution (2026-09-05):** re-verified after Software Engineer ran
`clang-format-18 -i` on both flagged files (commit `9589e10`, whitespace-
only, 28+/26- lines). `clang-format-18 --dry-run -Werror` clean on all
three files; rebuilt clean; full suite 80/80; `[CORE-205]` tag run
directly (`lottopicker_tests "[CORE-205]" -s`) shows 5/5 cases, 27/27
assertions. No functional change from the FAIL pass, so the earlier
hand-verified containment math still stands. Handed to
`agent:systems-engineer` for RTVM update.

CORE-205 (`BacktestEngine::run`, issue #20, 2026-09-05): first pass was a
FAIL, but only on the checked clang-format Coding Standard (docs/SDD.md),
not functionality — worth distinguishing from a real logic/test failure
when reading this later.

**What was right:** full suite 80/80 (75 prior + 5 new `[CORE-205]`
cases); hand-derived TP-CORE-205 part 1's containment counts (pool 9,
C(9,6)=84 total, split 20/45/18/1 at levels 3/4/5/6 via
`C(6,m)*C(3,6-m)`) independently and they matched exactly; part 2's
not-found case and both `BacktestError` throw cases correct; manual
end-to-end `lottopicker <cfg> --backtest <date>` against a hand-built
6-draw fixture (real era table, pool 53) confirmed real wiring: ~51s
(consistent with [[core_203_ranking_engine]]'s documented full-scan
cost), and a sample date with no training data reports its error
inline per-date rather than aborting the rest of the `--backtest` list.

**What failed:** `clang-format-18 --dry-run -Werror` (run from repo
root, so not the issue #19 discovery gotcha, see
[[data_out_300_ranked_list_structure]]) flagged real continuation-
indent/line-wrap violations in `src/BacktestEngine.cpp` (5 spots) and
`tests/BacktestEngineTest.cpp` (~15 spots) — `include/lottopicker/
BacktestEngine.h` itself was clean. This is the first CORE-2xx item
where clang-format actually failed after the fact (every prior pass
comment in this memory folder that checked it reported clean) — worth
explicitly re-running `clang-format-18 --dry-run -Werror` on every new/
changed file for future issues rather than assuming it'll be clean by
default now that one regression has happened.

**Two SWE-flagged open judgment calls to revisit at RTVM-update/
DATA-OUT-302 time** (not exercised by TP-CORE-205's assertions, so not
a test gate either way): percentile formula is against the retained
top-N envelope, not the full ~22.9M space; chance-expected containment
reuses CORE-206's `hypergeometricProbability` scaled by topN rather
than a true exact-match PMF. Re-check these land as intended once
DATA-OUT-302 (issue #22) fixes the report's real structure/formatting.
