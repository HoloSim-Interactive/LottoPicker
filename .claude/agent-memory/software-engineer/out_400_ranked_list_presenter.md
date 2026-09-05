---
name: out-400-ranked-list-presenter
description: RankedListPresenter design for OUT-400 (issue #21) -- pure-formatting console table, column-width/delimiter choices, and how it's tested without a real ranking run.
metadata:
  type: project
---

Issue #21 (branch `issue-21`) implemented OUT-400: `RankedListPresenter`
(`include/lottopicker/RankedListPresenter.h` / `src/RankedListPresenter.cpp`)
renders [[core_203_ranking_engine|RankingEngine]]'s
`std::vector<RankedCombo>` (DATA-OUT-300) as a human-readable console
table -- one header row (`Rank | Combination | Score`) plus exactly
one data row per entry, in the same order given (no re-sorting), per
docs/SDD.md's Interfaces & file formats wording ("a header row and
clearly delimited ... columns").

**Design: pure presentation, no computation dependency.** `print(std::ostream&,
const std::vector<RankedCombo>&)` takes an already-computed list and
any ostream -- no dependency on `ModelArtifact`/`RankingEngine`
themselves, so it's unit-testable against a hand-built fixture and an
`std::ostringstream`, matching TP-OUT-400's own wording ("given a
DATA-OUT-300 fixture with 5 entries") rather than requiring a full
ranking run in the test (which per [[core_203_ranking_engine]]'s
perf note takes ~43-50s wall-clock for a real pool-53 run).

**Column format chosen:** `" | "`-delimited, each column padded to the
widest value in that column (header included) -- picked over a fixed
assumed width because scores have no fixed a priori magnitude/decimal
length. Score printed with 6 fixed decimal digits (a human-facing
precision choice, unrelated to DATA-OUT-301's bit-for-bit round-trip
precision requirement for the persisted model file -- don't conflate
the two when touching either).

**Wired into `main()`:** the ranking-path branch's previous ad hoc
`rank:/combo/score` loop is now a single
`RankedListPresenter::print(std::cout, ranked)` call.

**How to apply:** if OUT-401 (backtest console report, DATA-OUT-302)
lands later, consider whether it wants a similar dedicated presenter
class following this same pure-formatting pattern (its own struct
plus its own presenter), rather than ad hoc printing in `main()` --
OUT-401 has an extra formatting wrinkle already flagged in RTVM (the
"not found in top-N" case must render distinctly from a numeric rank,
never a placeholder like `-1`), which a dedicated class can unit-test
cleanly the same way this one does for OUT-400's ties/ordering.
