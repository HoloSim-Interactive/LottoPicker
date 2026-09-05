---
name: data-out-302-backtest-report-structure
description: DATA-OUT-302 (issue #22) -- BacktestResult (CORE-205) already was the report-row structure; only a dedicated 3-year test + header-comment update were needed, third occurrence of the DATA-OUT-30x pattern.
metadata:
  type: project
---

Issue #22 is the third case (after DATA-OUT-300/issue #19 and
DATA-OUT-301/issue #18, see [[data_out_300_ranked_list_structure]]) of
an RTVM item whose structure was already delivered as a side effect of
an earlier item -- here, [[core_205_backtest_engine]]'s `BacktestResult`
struct. No new struct or collection type was needed: the full backtest
report for N sample dates is just `std::vector<BacktestResult>`, one
entry per date, mirroring how DATA-OUT-300 is just
`std::vector<RankedCombo>` with no dedicated container type of its own.

**What was actually done:**
1. Updated `BacktestEngine.h`'s header comment on `BacktestResult` to
   state explicitly that it *is* DATA-OUT-302's report-row structure
   (previously it said "not yet... that's issue #22"), mirroring
   `RankingEngine.h`'s explicit DATA-OUT-300 comment.
2. Refreshed `main.cpp`'s stale comment that still called DATA-OUT-302
   "not-yet-implemented".
3. Added a `[DATA-OUT-302][CORE-205]`-tagged test in
   `tests/BacktestEngineTest.cpp` matching TP-DATA-OUT-302's exact
   fixture ("3 synthetic years... one row per year, all fields
   populated, consistent with TP-CORE-205's results"): builds a
   3-sampled-year history reusing TP-CORE-205 part 1's pool-size-9
   full-space-retention fixture shape, loops `BacktestEngine::run` per
   year into a `std::vector<BacktestResult>`, and asserts each row
   reproduces the same hand-verified 20/45/18/1 containment breakdown
   independently (full-space retention makes that breakdown depend
   only on draw-size/pool-size, not which specific numbers are drawn --
   worth remembering if a similar "does the number identity matter"
   question comes up again for containment-style fixtures).

**Reusable lesson (reinforcing [[data_out_300_ranked_list_structure]]):**
when a dependent RTVM item's requirement is already satisfied by an
earlier item's production struct, check for a *dedicated, correctly-
tagged test matching the test procedure's exact fixture* before
concluding there's nothing to do. Here that also meant updating two
stale "not yet implemented" comments left behind by the earlier issue
that explicitly deferred this item -- worth grepping for the dependent
RTVM ID in comments (not just structs) when picking up a "structure
already exists" issue.
