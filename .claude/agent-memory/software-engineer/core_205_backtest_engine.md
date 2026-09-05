---
name: core-205-backtest-engine
description: BacktestEngine design (issue #20) — truncation/era-pool wiring, and two documented formula judgment calls (percentile, chance-expected containment) flagged for Systems Engineer confirmation.
metadata:
  type: project
---

CORE-205's `BacktestEngine::run` (issue #20) truncates an already-
ingested/era-tagged `history` to strictly-before a sample date, reuses
`ModelStore::buildArtifact` (now public, not just `loadOrBuild`'s
private helper — see [[core_204_model_store]]) to build a throwaway,
unpersisted model from that truncated set, then calls
`RankingEngine::rank` bounded to the **era-appropriate pool size for
the sample date itself** (`EraTable::poolSizeForDate`), not the
model's own latest-tagged pool — matters when a sample date sits near
an era boundary. `poolSizeForDate`'s `eras` table parameter (already
existed for `EraTagger::tag`/`poolSizeForDate` testability) is threaded
all the way through `BacktestEngine::run` too, specifically so tests
can pin a tiny synthetic pool (e.g. size 9, giving `C(9,6)=84` — a
fully hand-enumerable space) instead of paying RankingEngine's real
~55s full-scan cost (pool 53) per test case.

**Two judgment calls I made that are NOT fixed by docs/SDD.md or
docs/RTVM.md, and NOT exercised by TP-CORE-205's own assertions** —
flagged in the issue #20 hand-off comment for Systems Engineer to
confirm or override:
1. **Percentile** = `100 * (topN - rank + 1) / topN`, i.e. relative to
   the *retained topN envelope*, not the full ~13.9M/22.9M-combination
   space (CORE-203 never materializes that space, so there's no true
   global rank outside topN to compute a full-space percentile from).
2. **Chance-expected containment** reuses CORE-206's
   `PoolSizeNormalizer::hypergeometricProbability(level, poolMax)`
   directly (scaled by topN) rather than deriving a new "exactly
   `level` of 6 match" hypergeometric PMF — those are related but
   distinct formulas; I chose reusing already-Verified CORE-206 math
   over introducing new, untested math, since the RTVM item's own
   wording names "CORE-206's hypergeometric baseline" directly.

If Systems Engineer/Test Engineer push back on either, the fix is
localized to `BacktestEngine.cpp`'s last two computed fields — no
structural change needed elsewhere.

Also: `BacktestResult` is deliberately NOT DATA-OUT-302's formal report
row — that's the separate, dependent issue #22 (same relationship
`RankedCombo`/CORE-203 had to `DATA-OUT-300` before issue #19 added its
own dedicated test).
