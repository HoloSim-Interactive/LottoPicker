---
name: core-203-ranking-engine
description: How TP-CORE-203 (full-space top-N retention) was verified, including a real end-to-end timing data point and the SDD v1/v2 flag to watch for at RTVM-update time.
metadata:
  type: project
---

CORE-203 (issue #17, 2026-09-05, passed): 74/74 (up from 59/59 at
CORE-204/issue #16). Verification combined the standard ctest run with
independent manual checks since TP-CORE-203 part 2 is Analysis, not a
runtime assertion:

- Read `RankingEngine::rank`'s source directly (not just trusted the
  SE's summary) to confirm the min-heap/nested-loop design never holds
  more than one `combo` plus the fixed-size heap — satisfies "full
  space never materialized" myself rather than taking it on faith.
- Ran the built CLI end-to-end against an ad-hoc 5-draw fixture CSV
  (same recipe as [[core_204_model_store]]) with real default pool
  (1-53): first run ~55s wall-clock (rebuilt), second run ~56s
  (reused model, ranking re-scanned as expected) — consistent with
  SE's ~43s figure and the documented `O(C(n,6) log topN)` class; not
  a hang. **Budget ~1 minute per real full-pool run when manually
  verifying future issues that touch ranking** (e.g. CORE-205's
  backtest, OUT-400) — don't set a tighter timeout and mistake it for
  a failure.
- Confirmed exactly `top_n` results, no duplicate combos, exit 0. Top
  5 of 5 historical draws ranked highest as expected. Some ties
  appeared among lower ranks at real scale (expected/correct per RTVM
  — only unique *combinations* required, not unique *scores*); the
  unit test's "strictly descending" check uses a deliberately
  tie-free powers-of-two fixture instead, which is the right place for
  that assertion, not the real-data manual run.

**Open flag for Systems Engineer at RTVM-update time (not a test
failure):** `docs/SDD.md` line ~120 still says `LOTTOPICKER_MODEL v1`
with no `baseline_cooc` line — SE bumped the on-disk format to v2 for
this issue and already flagged it in their hand-off comment, but it's
still unresolved in the doc as of this pass. Worth confirming SE's
follow-up (or SysE) actually updates it before/alongside marking
CORE-203/DATA-OUT-301 Verified.
